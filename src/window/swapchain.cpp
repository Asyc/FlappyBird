#include "swapchain.hpp"

#include <algorithm>

#include "context/render.hpp"

Swapchain::Swapchain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface, const Queue& graphics, const Queue& present, const FormatSelector& formatSelector,
                     const PresentModeSelector& presentModeSelector)
        : m_Swapchain(vk::SwapchainKHR(nullptr), vk::ObjectDestroy<vk::Device, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>(device)),
          m_PresentMode(std::invoke(presentModeSelector, physicalDevice.getSurfacePresentModesKHR(surface))),
          m_Format(std::invoke(formatSelector, physicalDevice.getSurfaceFormatsKHR(surface))),
          m_SharingMode(graphics == present ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent),
          m_QueueFamilyIndices({graphics.m_Index, present.m_Index}),
          m_GraphicsQueue(&graphics),
          m_PresentQueue(&present),
          m_CurrentFlight(0) {
    vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    m_ImageCount = std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount, capabilities.maxImageCount);

    vk::CommandPoolCreateInfo commandPoolCreateInfo({vk::CommandPoolCreateFlagBits::eResetCommandBuffer}, graphics.m_Index);
    m_CommandPool = device.createCommandPoolUnique(commandPoolCreateInfo);

    createSwapchain(physicalDevice, surface);
}

Swapchain::~Swapchain() {
    if (!m_Swapchain) return;
    m_Swapchain.getOwner().waitIdle();
}

void Swapchain::createSwapchain(vk::PhysicalDevice device, vk::SurfaceKHR surface) {
    vk::SurfaceCapabilitiesKHR capabilities = device.getSurfaceCapabilitiesKHR(surface);

    m_Extent = capabilities.currentExtent;

    vk::SwapchainCreateInfoKHR createInfo(
            {},
            surface,
            m_ImageCount,
            m_Format.format,
            m_Format.colorSpace,
            m_Extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            m_SharingMode,
            m_QueueFamilyIndices,
            capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            m_PresentMode,
            VK_TRUE,
            *m_Swapchain
    );

    m_Swapchain = m_Swapchain.getOwner().createSwapchainKHRUnique(createInfo);

    vk::AttachmentDescription attachmentDescription({}, m_Format.format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
                                                    vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference attachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::SubpassDescription subpassDescription({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &attachmentReference);
    vk::SubpassDependency subpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, vk::AccessFlagBits::eColorAttachmentWrite);
    vk::RenderPassCreateInfo renderPassCreateInfo({}, 1, &attachmentDescription, 1, &subpassDescription, 1, &subpassDependency);

    m_RenderPass = m_Swapchain.getOwner().createRenderPassUnique(renderPassCreateInfo);

    std::vector<vk::Image> images = m_Swapchain.getOwner().getSwapchainImagesKHR(*m_Swapchain);
    m_Images.resize(images.size());
    for (int i = 0; i < m_Images.size(); i++) {
        m_Images[i] = Image(i, m_Swapchain.getOwner(), *m_RenderPass, images[i], m_Format.format, m_Extent.width, m_Extent.height);
    }

    m_Flights.resize(4);
    for (auto& m_Flight : m_Flights) {
        m_Flight = ImageFlight(m_Swapchain.getOwner(), *m_CommandPool);
    }
}

void Swapchain::nextImage() {
    if (m_Flights.size() == ++m_CurrentFlight) m_CurrentFlight = 0;
    auto& flight = m_Flights[m_CurrentFlight];

    uint32_t imageIndex = m_Swapchain.getOwner().acquireNextImageKHR(*m_Swapchain, UINT64_MAX, *flight.m_ImageReady, {});
    flight.m_BoundImage = &m_Images[imageIndex];

    m_Swapchain.getOwner().waitForFences(1, &*flight.m_RenderFinishedFence, VK_FALSE, UINT64_MAX);

    flight.m_CommandBuffer->begin(vk::CommandBufferBeginInfo());

    vk::ClearValue clearColorValue(std::array<float, 4>{});
    flight.m_CommandBuffer->beginRenderPass(vk::RenderPassBeginInfo(*m_RenderPass, *flight.m_BoundImage->m_Framebuffer, vk::Rect2D({}, m_Extent), 1, &clearColorValue), vk::SubpassContents::eSecondaryCommandBuffers);
}

void Swapchain::presentImage() {
    auto& flight = m_Flights[m_CurrentFlight];

    flight.m_CommandBuffer->endRenderPass();
    flight.m_CommandBuffer->end();

    m_Swapchain.getOwner().resetFences(1, &*flight.m_RenderFinishedFence);

    vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo(1, &*flight.m_ImageReady, &flags, 1, &*flight.m_CommandBuffer, 1, &*flight.m_RenderFinished);
    m_GraphicsQueue->m_Queue.submit(submitInfo, *flight.m_RenderFinishedFence);

    m_PresentQueue->m_Queue.presentKHR(vk::PresentInfoKHR(1, &*flight.m_RenderFinished, 1, &*m_Swapchain, &flight.m_BoundImage->m_Index));
}

vk::Extent2D Swapchain::getExtent() const {
    return m_Extent;
}

vk::Format Swapchain::getFormat() const {
    return m_Format.format;
}

Swapchain::Image::Image(uint32_t index, vk::Device device, vk::RenderPass renderPass, vk::Image image, vk::Format format, uint32_t width, uint32_t height) : m_Index(index) {
    vk::ImageViewCreateInfo imageViewCreateInfo(
            {},
            image,
            vk::ImageViewType::e2D,
            format,
            {},
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
    );

    m_ImageView = device.createImageViewUnique(imageViewCreateInfo);

    vk::FramebufferCreateInfo framebufferCreateInfo({}, renderPass, 1, &*m_ImageView, width, height, 1);
    m_Framebuffer = device.createFramebufferUnique(framebufferCreateInfo);
}

Swapchain::ImageFlight::ImageFlight(vk::Device device, vk::CommandPool pool) : m_BoundImage(nullptr) {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo(pool, vk::CommandBufferLevel::ePrimary, 1);
    m_CommandBuffer = std::move(device.allocateCommandBuffersUnique(commandBufferAllocateInfo)[0]);

    vk::SemaphoreCreateInfo semaphoreCreateInfo;
    m_ImageReady = device.createSemaphoreUnique(semaphoreCreateInfo);
    m_RenderFinished = device.createSemaphoreUnique(semaphoreCreateInfo);

    vk::FenceCreateInfo fenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
    m_RenderFinishedFence = device.createFenceUnique(fenceCreateInfo);
}

vk::SurfaceFormatKHR Swapchain::selectFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
    return formats[0];
}

vk::PresentModeKHR Swapchain::selectPresentMode(const std::vector<vk::PresentModeKHR>& presentModes) {
    return vk::PresentModeKHR::eImmediate;
}

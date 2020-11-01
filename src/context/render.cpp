#include "render.hpp"

#include <array>
#include <optional>

#include <GLFW/glfw3.h>

RenderContext::RenderContext(const ApplicationInfo& info, Window& window, const DeviceFunction& deviceSelector) {
    uint32_t version = VK_MAKE_VERSION(info.version.major, info.version.minor, info.version.patch);
    vk::ApplicationInfo applicationInfo(info.title.data(), version, "No-Engine", version, VK_API_VERSION_1_2);

    uint32_t extensionCount;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    vk::InstanceCreateInfo createInfo({}, &applicationInfo, 0, nullptr, extensionCount, extensions);
#ifndef NDEBUG
    std::array<const char*, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();
#endif
    m_Instance = vk::createInstanceUnique(createInfo);
    m_Surface = window.createWindowSurface(*m_Instance);
    m_PhysicalDevice = std::invoke(deviceSelector, m_Instance->enumeratePhysicalDevices());


    std::optional<uint32_t> graphics, present;
    uint32_t index = 0;
    for (const auto& queue : m_PhysicalDevice.getQueueFamilyProperties()) {
        if (!graphics.has_value() && queue.queueFlags & vk::QueueFlagBits::eGraphics) graphics = index;
        if (!present.has_value() && m_PhysicalDevice.getSurfaceSupportKHR(index, *m_Surface)) present = index;
        if (graphics.has_value() && present.has_value()) break;

        index++;
    }

    float priority = 1.0f;
    std::array<vk::DeviceQueueCreateInfo, 2> queues {
        vk::DeviceQueueCreateInfo({}, graphics.value(), 1, &priority),
        vk::DeviceQueueCreateInfo({}, present.value(), 1, &priority),
    };

    std::array<const char*, 1> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::PhysicalDeviceFeatures features;
    vk::DeviceCreateInfo deviceCreateInfo({}, graphics == present ? 1 : 2, queues.data(), 0, nullptr, deviceExtensions.size(), deviceExtensions.data(), &features);
    m_Device = m_PhysicalDevice.createDeviceUnique(deviceCreateInfo);
    m_GraphicsQueue = Queue{graphics.value(), m_Device->getQueue(graphics.value(), 0)};
    m_PresentQueue = Queue{present.value(), m_Device->getQueue(present.value(), 0)};

    m_Swapchain = Swapchain(m_PhysicalDevice, *m_Device, *m_Surface, m_GraphicsQueue, m_PresentQueue);

    window.setMetadata(this);
    window.setResizeCallback([](Window& window, uint32_t width, uint32_t height){
        auto* context = reinterpret_cast<RenderContext*>(window.getMetadata());
        context->m_Device->waitIdle();
        context->m_Swapchain.createSwapchain(context->m_PhysicalDevice, *context->m_Surface);
    });
}

Shader RenderContext::createShader(const std::string_view& path) {
    return Shader(*m_Device, path);
}

Swapchain& RenderContext::getSwapchain() {
    return m_Swapchain;
}

vk::PhysicalDevice RenderContext::selectPhysicalDevice(const std::vector<vk::PhysicalDevice>& devices) {
    return devices[0];
}

bool Queue::operator==(const Queue& rhs) const {
    return m_Index == rhs.m_Index;
}

bool Queue::operator!=(const Queue& rhs) const {
    return m_Index != rhs.m_Index;
}

const vk::Queue* Queue::operator->() const {
    return &m_Queue;
}
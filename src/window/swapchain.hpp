#ifndef FLAPPYBIRD_SWAPCHAIN_HPP
#define FLAPPYBIRD_SWAPCHAIN_HPP

#include <array>
#include <functional>
#include <vector>

#include <vulkan/vulkan.hpp>

class Queue;

class Swapchain {
public:
    using FormatSelector = std::function<vk::SurfaceFormatKHR(const std::vector<vk::SurfaceFormatKHR>&)>;
    using PresentModeSelector = std::function<vk::PresentModeKHR(const std::vector<vk::PresentModeKHR>&)>;

    Swapchain() = default;
    Swapchain(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface, const Queue& graphics, const Queue& present, uint32_t maxImagesInFlight = 4, const FormatSelector& formatSelector = selectFormat,
              const PresentModeSelector& presentModeSelector = selectPresentMode);
    ~Swapchain();

    Swapchain& operator=(const Swapchain& rhs) = delete;
    Swapchain& operator=(Swapchain&& rhs) = default;

    void createSwapchain(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    void nextImage();
    void presentImage();

    [[nodiscard]] vk::Extent2D getExtent() const;
    [[nodiscard]] vk::Format getFormat() const;
private:
    struct Image {
        Image() = default;
        Image(uint32_t index, vk::Device device, vk::RenderPass renderPass, vk::Image image, vk::Format format, uint32_t width, uint32_t height);
        uint32_t m_Index;
        vk::UniqueImageView m_ImageView;
        vk::UniqueFramebuffer m_Framebuffer;
    };

    struct ImageFlight {
        ImageFlight() = default;
        explicit ImageFlight(vk::Device device, vk::CommandPool pool);

        vk::UniqueCommandBuffer m_CommandBuffer;
        vk::UniqueSemaphore m_ImageReady, m_RenderFinished;
        vk::UniqueFence m_RenderFinishedFence;

        Image* m_BoundImage;
    };

    vk::UniqueSwapchainKHR m_Swapchain;

    vk::PresentModeKHR m_PresentMode;
    vk::SurfaceFormatKHR m_Format;
    vk::Extent2D m_Extent;
    uint32_t m_ImageCount;

    vk::SharingMode m_SharingMode;
    std::array<uint32_t, 2> m_QueueFamilyIndices;

    vk::UniqueRenderPass m_RenderPass;
    vk::UniqueCommandPool m_CommandPool;

    const Queue* m_GraphicsQueue;
    const Queue* m_PresentQueue;

    std::vector<Image> m_Images;
    std::vector<ImageFlight> m_Flights;
    size_t m_CurrentFlight;

    static vk::SurfaceFormatKHR selectFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
    static vk::PresentModeKHR selectPresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
};


#endif //FLAPPYBIRD_SWAPCHAIN_HPP

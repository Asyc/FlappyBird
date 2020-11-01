#ifndef FLAPPYBIRD_RENDER_HPP
#define FLAPPYBIRD_RENDER_HPP

#include <cstdint>
#include <functional>
#include <string_view>

#include <vulkan/vulkan.hpp>

#include "window/swapchain.hpp"
#include "window/window.hpp"

struct ApplicationInfo {
    struct {
        uint8_t major, minor, patch;
    } version;

    const std::string_view& title;
};

struct Queue {
    uint32_t m_Index;
    vk::Queue m_Queue;

    bool operator==(const Queue& rhs) const;
    bool operator!=(const Queue& rhs) const;
    const vk::Queue* operator->() const;
};

class RenderContext {
public:
    using DeviceFunction = std::function<vk::PhysicalDevice(const std::vector<vk::PhysicalDevice>&)>;

    RenderContext(const ApplicationInfo& info, const Window& window, const DeviceFunction& deviceSelector = selectPhysicalDevice);

    Swapchain& getSwapchain();
private:
    vk::UniqueInstance m_Instance;
    vk::UniqueSurfaceKHR m_Surface;

    vk::PhysicalDevice m_PhysicalDevice;
    vk::UniqueDevice m_Device;
    Queue m_GraphicsQueue, m_PresentQueue;

    Swapchain m_Swapchain;


    static vk::PhysicalDevice selectPhysicalDevice(const std::vector<vk::PhysicalDevice>& devices);
};

#endif //FLAPPYBIRD_RENDER_HPP

#ifndef FLAPPYBIRD_RENDER_HPP
#define FLAPPYBIRD_RENDER_HPP

#include <cstdint>
#include <functional>
#include <string_view>

#include <vulkan/vulkan.hpp>

#include "window/window.hpp"

struct ApplicationInfo {
    struct {
        uint8_t major, minor, patch;
    } version;

    const std::string_view& title;
};

class RenderContext {
public:
    using DeviceFunction = std::function<vk::PhysicalDevice(const std::vector<vk::PhysicalDevice>&)>;
    static vk::PhysicalDevice selectPhysicalDevice(const std::vector<vk::PhysicalDevice>& devices);

    RenderContext(const ApplicationInfo& info, const Window& window, const DeviceFunction& deviceSelector = selectPhysicalDevice);
private:
    vk::UniqueInstance m_Instance;
    vk::UniqueSurfaceKHR m_Surface;

    vk::PhysicalDevice m_PhysicalDevice;
    vk::UniqueDevice m_Device;

    struct Queue {
        uint32_t index;
        vk::Queue m_Queue;

        bool operator==(const Queue& rhs) const;
        vk::Queue* operator->();
    } m_GraphicsQueue, m_PresentQueue;
};

#endif //FLAPPYBIRD_RENDER_HPP

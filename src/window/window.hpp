#ifndef FLAPPYBIRD_WINDOW_HPP
#define FLAPPYBIRD_WINDOW_HPP

#include <cstdint>
#include <string_view>

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

class Window {
public:
    using ResizeCallback = std::function<void(Window& window, uint32_t, uint32_t)>;

    Window(uint16_t width, uint16_t height, const std::string_view& title);
    Window(const Window& rhs) = delete;
    Window(Window&& rhs) noexcept;
    ~Window();

    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) noexcept;

    [[nodiscard]] vk::UniqueSurfaceKHR createWindowSurface(vk::Instance instance) const;

    void setResizeCallback(ResizeCallback callback);

    void setMetadata(void* metadata);
    [[nodiscard]] void* getMetadata() const;

    [[nodiscard]] bool shouldClose() const;
    [[nodiscard]] GLFWwindow* getWindowHandle() const;

    static void pollEvents();
private:
    GLFWwindow* m_Window;
    void* m_Metadata;

    ResizeCallback m_ResizeCallback;

    static size_t s_InitCounter;
};

#endif //FLAPPYBIRD_WINDOW_HPP

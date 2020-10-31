#ifndef FLAPPYBIRD_WINDOW_HPP
#define FLAPPYBIRD_WINDOW_HPP

#include <cstdint>
#include <string_view>

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

class Window {
public:
    Window(uint16_t width, uint16_t height, const std::string_view& title);
    Window(const Window& rhs) = delete;
    Window(Window&& rhs) noexcept;
    ~Window();

    Window& operator=(const Window& other) = delete;
    Window& operator=(Window&& other) noexcept;

    [[nodiscard]] vk::UniqueSurfaceKHR createWindowSurface(vk::Instance instance) const;

    [[nodiscard]] bool shouldClose() const;
    [[nodiscard]] GLFWwindow* getWindowHandle() const;

    static void pollEvents();
private:
    GLFWwindow* m_Window;

    static size_t s_InitCounter;
};

#endif //FLAPPYBIRD_WINDOW_HPP

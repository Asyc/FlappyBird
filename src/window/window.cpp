#include "window.hpp"

#include <stdexcept>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

size_t Window::s_InitCounter = 0;

Window::Window(uint16_t width, uint16_t height, const std::string_view& title) {
    if (s_InitCounter++ == 0 && glfwInit() != GLFW_TRUE) throw std::runtime_error("failed to initialize glfw");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
}

Window::Window(Window&& rhs) noexcept {
    m_Window = rhs.m_Window;
    rhs.m_Window = nullptr;
}

Window::~Window() {
    if (m_Window == nullptr) return;
    glfwDestroyWindow(m_Window);
    if (--s_InitCounter == 0) glfwTerminate();
}

Window& Window::operator=(Window&& rhs) noexcept {
    if (this != &rhs) return *this;

    m_Window = rhs.m_Window;
    rhs.m_Window = nullptr;

    return *this;
}

vk::UniqueSurfaceKHR Window::createWindowSurface(vk::Instance instance) const {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

    return vk::UniqueSurfaceKHR(surface, vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>(instance));
}

GLFWwindow* Window::getWindowHandle() const {
    return m_Window;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_Window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

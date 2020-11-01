#include "window.hpp"

#include <stdexcept>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

size_t Window::s_InitCounter = 0;

Window::Window(uint16_t width, uint16_t height, const std::string_view& title) : m_Metadata(nullptr) {
    if (s_InitCounter++ == 0 && glfwInit() != GLFW_TRUE) throw std::runtime_error("failed to initialize glfw");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window, this);
}

Window::Window(Window&& rhs) noexcept {
    m_Window = rhs.m_Window;
    m_Metadata = rhs.m_Metadata;

    rhs.m_Window = nullptr;
    rhs.m_Metadata = nullptr;
}

Window::~Window() {
    if (m_Window == nullptr) return;
    glfwDestroyWindow(m_Window);
    if (--s_InitCounter == 0) glfwTerminate();
}

Window& Window::operator=(Window&& rhs) noexcept {
    if (this != &rhs) return *this;

    m_Window = rhs.m_Window;
    m_Metadata = rhs.m_Metadata;

    rhs.m_Window = nullptr;
    rhs.m_Metadata = nullptr;

    return *this;
}

vk::UniqueSurfaceKHR Window::createWindowSurface(vk::Instance instance) const {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

    return vk::UniqueSurfaceKHR(surface, vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>(instance));
}

void Window::setResizeCallback(Window::ResizeCallback callback) {
    m_ResizeCallback = std::move(callback);

    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* handle, int width, int height){
        auto* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
        if (window->m_ResizeCallback != nullptr) {
            std::invoke(window->m_ResizeCallback, *window, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        }
    });
}

void Window::setMetadata(void* metadata) {
    m_Metadata = metadata;
}

void* Window::getMetadata() const {
    return m_Metadata;
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

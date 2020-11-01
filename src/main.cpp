#include <chrono>
#include <iostream>

#include "window/window.hpp"
#include "context/render.hpp"

int main() {
    Window window(1920, 1080, "Window");

    RenderContext context(ApplicationInfo{{0, 1, 0}, ""}, window);

    auto last = std::chrono::high_resolution_clock::now();
    size_t updates = 0;

    while (!window.shouldClose()) {
        Window::pollEvents();

        context.getSwapchain().nextImage();
        context.getSwapchain().presentImage();
        updates++;

        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last).count() >= 1) {
            std::cout << "FPS: " << updates << '\n';
            last = now;
            updates = 0;
        }
    }
}

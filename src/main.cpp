#include "window/window.hpp"
#include "context/render.hpp"

int main() {
    Window window(1920, 1080, "Window");

    RenderContext context(ApplicationInfo{{0, 1, 0}, ""}, window);

    while (!window.shouldClose()) {
        Window::pollEvents();
    }
}

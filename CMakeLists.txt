cmake_minimum_required(VERSION 3.18)
project(FlappyBird)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libs/glfw)

include_directories(src)

add_executable(FlappyBird src/main.cpp src/window/window.cpp src/window/window.hpp src/context/render.cpp src/context/render.hpp)
target_link_libraries(FlappyBird PUBLIC Vulkan::Vulkan glfw)
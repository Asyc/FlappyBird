cmake_minimum_required(VERSION 3.18)
project(FlappyBird)
find_package(Vulkan REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libs/glfw)
add_subdirectory(libs/rapidjson)

include_directories(src)

set(SOURCES
        src/main.cpp
        src/window/window.cpp src/window/window.hpp
        src/context/render.cpp src/context/render.hpp
        src/window/swapchain.cpp src/window/swapchain.hpp
        src/render/pipeline.cpp src/render/pipeline.hpp
        src/render/shader.cpp src/render/shader.hpp
        src/io/file.cpp src/io/file.hpp
        )

add_executable(FlappyBird ${SOURCES})
target_link_libraries(FlappyBird PUBLIC Vulkan::Vulkan glfw)
target_include_directories(FlappyBird PRIVATE libs/rapidjson/include)

set(GLSL_SOURCE_FILES
        assets/shaders/standard.vert
        assets/shaders/standard.frag
        )

# Compiling GLSL -> SPIRV

foreach (FILE ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${FILE} NAME)
    set(TARGET "${FILE_NAME}.spv")

    add_custom_command(
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders
            OUTPUT ${TARGET}
            COMMAND glslangvalidator -V ${FILE_NAME} -o ${TARGET}
            DEPENDS ${FILE}
    )
    list(APPEND BINARY_SHADERS ${TARGET})
endforeach ()

add_custom_target(Shaders DEPENDS ${BINARY_SHADERS})
add_dependencies(FlappyBird Shaders)
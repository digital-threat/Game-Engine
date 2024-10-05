cmake_minimum_required(VERSION 3.29)
project(Engine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

add_library(${PROJECT_NAME} src/engine.cpp)

target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            glm
            glfw
            Vulkan::Vulkan
        PRIVATE

)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
            glm
            glfw
            Vulkan::Vulkan
        PRIVATE
)

cmake_minimum_required(VERSION 3.29)
project(Engine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

add_library(${PROJECT_NAME} src/engine.cpp)

set(source "${CMAKE_SOURCE_DIR}/shaders")
set(destination "${CMAKE_CURRENT_BINARY_DIR}/shaders")

FILE(COPY ${CMAKE_SOURCE_DIR}/engine/shaders/vert.spv DESTINATION "${CMAKE_BINARY_DIR}/engine/shaders")
FILE(COPY ${CMAKE_SOURCE_DIR}/engine/shaders/frag.spv DESTINATION "${CMAKE_BINARY_DIR}/engine/shaders")

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

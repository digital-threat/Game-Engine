cmake_minimum_required(VERSION 3.29)
project(engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/vk-bootstrap)
add_subdirectory(vendor/vma)
add_subdirectory(vendor/rapidobj)

# Dear ImGui library
file(GLOB IMGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)
set_target_properties(imgui PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/vendor/imgui)

# Engine library
file(GLOB_RECURSE ENGINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES})

add_compile_definitions(PLATFORM_WINDOWS)
add_compile_definitions(GRAPHICS_API_VULKAN)
add_compile_definitions(BUILD_DLL)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb>
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        Vulkan::Vulkan
        imgui
        glfw
        glm::glm-header-only
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
        rapidobj
)
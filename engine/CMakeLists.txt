cmake_minimum_required(VERSION 3.29)
project(engine VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

# Dear ImGui library
file(GLOB IMGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
target_link_libraries(imgui PUBLIC Vulkan::Vulkan glfw)

# Engine library
file(GLOB_RECURSE ENGINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCES})


target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
            imgui
            Vulkan::Vulkan
            glfw
            glm::glm-header-only
)


set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

file(GLOB_RECURSE SHADER_FILES "${SHADER_SOURCE_DIR}/*.vert" "${SHADER_SOURCE_DIR}/*.frag")

# Shader files must not have the same name, even if they have different extensions
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(SHADER_BINARY ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv)

    add_custom_command(
            OUTPUT ${SHADER_BINARY}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_BINARY}
            DEPENDS ${SHADER}
            COMMENT "Compiling ${SHADER} to ${SHADER_BINARY}"
            VERBATIM
    )

    list(APPEND SHADER_BINARIES ${SHADER_BINARY})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SHADER_BINARIES})

add_dependencies(${PROJECT_NAME} Shaders)
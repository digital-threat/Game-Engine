cmake_minimum_required(VERSION 3.29)
project(Game VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Address Sanitizer
# target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
# target_link_directories(${PROJECT_NAME} PRIVATE $ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/lib/clang/19/lib/windows)
# target_link_libraries(${PROJECT_NAME} PRIVATE engine clang_rt.asan_dynamic-x86_64 clang_rt.asan_dynamic_runtime_thunk-x86_64)
# target_link_options(${PROJECT_NAME} PRIVATE /wholearchive:clang_rt.asan_dynamic_runtime_thunk-x86_64.lib)
# add_compile_definitions(_DISABLE_STRING_ANNOTATION=1 _DISABLE_VECTOR_ANNOTATION=1)

target_link_libraries(${PROJECT_NAME} engine)
target_include_directories(${PROJECT_NAME} PRIVATE ../engine)
target_include_directories(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

file(GLOB_RECURSE SHADER_FILES ${SHADER_SOURCE_DIR}/*.vert ${SHADER_SOURCE_DIR}/*.frag ${SHADER_SOURCE_DIR}/*.comp)

# Shader files must not have the same name, even if they have different extensions
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(SHADER_BINARY ${SHADER_BINARY_DIR}/${SHADER_NAME}.spv)

    add_custom_command(
            OUTPUT ${SHADER_BINARY}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_BINARY}
            DEPENDS ${SHADER}
            COMMENT Compiling ${SHADER} to ${SHADER_BINARY}
            VERBATIM
    )

    list(APPEND SHADER_BINARIES ${SHADER_BINARY})
endforeach()

target_sources(${PROJECT_NAME} PRIVATE ${SHADER_BINARIES})


# Copy textures
set(TEXTURES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/textures)
set(TEXTURES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets/textures)

file(MAKE_DIRECTORY ${TEXTURES_BINARY_DIR})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${TEXTURES_SOURCE_DIR}
        ${TEXTURES_BINARY_DIR}
        COMMENT Copying textures to build directory
)

# Copy meshes
set(MESHES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets/meshes)
set(MESHES_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/assets/meshes)

file(MAKE_DIRECTORY ${MESHES_BINARY_DIR})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${MESHES_SOURCE_DIR}
        ${MESHES_BINARY_DIR}
        COMMENT Copying meshes to build directory
)

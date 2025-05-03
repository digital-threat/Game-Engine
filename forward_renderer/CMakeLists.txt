cmake_minimum_required(VERSION 3.29)
project(forward_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

add_compile_definitions(PLATFORM_WINDOWS)
add_compile_definitions(GRAPHICS_API_VULKAN)

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

# Compile shaders and output result to build folder
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

file(GLOB_RECURSE SHADER_FILES
        ${SHADER_SOURCE_DIR}/*.vert
        ${SHADER_SOURCE_DIR}/*.frag
        ${SHADER_SOURCE_DIR}/*.comp
        ${SHADER_SOURCE_DIR}/*.rchit
        ${SHADER_SOURCE_DIR}/*.rmiss
        ${SHADER_SOURCE_DIR}/*.rgen
        ${SHADER_SOURCE_DIR}/*.rahit
)

foreach (SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_filename_component(SHADER_EXT ${SHADER} EXT)
    set(SHADER_BINARY ${SHADER_BINARY_DIR}/${SHADER_NAME}${SHADER_EXT}.spv)

    add_custom_command(
            OUTPUT ${SHADER_BINARY}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} --target-env=vulkan1.3 ${SHADER} -o ${SHADER_BINARY}
            DEPENDS ${SHADER}
            COMMENT Compiling ${SHADER} to ${SHADER_BINARY}
            VERBATIM
    )

    list(APPEND SHADER_BINARIES ${SHADER_BINARY})
endforeach ()

target_sources(${PROJECT_NAME} PRIVATE ${SHADER_BINARIES})

# Copy assets to build directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/models)
set(BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/models)
file(MAKE_DIRECTORY ${BINARY_DIR})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SOURCE_DIR} ${BINARY_DIR}
        COMMENT Copying assets to build directory
)

cmake_minimum_required(VERSION 3.29)
project(Game VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} Engine)
target_include_directories(${PROJECT_NAME} PRIVATE ../engine)

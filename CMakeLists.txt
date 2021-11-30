cmake_minimum_required(VERSION 3.5.1)

# set the project name 
project(CalculateSqrt)

# add the executable 
add_executable(CalculateSqrt src/main.cpp)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(include/llhttp)

add_subdirectory(include/uv)

target_link_libraries(CalculateSqrt PUBLIC include/llhttp)

target_link_libraries(CalculateSqrt PUBLIC include/uv)

target_include_directories(CalculateSqrt PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/include/llhttp"
                          "${PROJECT_SOURCE_DIR}/include/uv"
                          )
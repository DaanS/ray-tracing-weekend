cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.16)

project(RayTracingWeekend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory("${CMAKE_SOURCE_DIR}/test")

find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(main src/main.cpp)
target_link_libraries(main Threads::Threads nlohmann_json::nlohmann_json)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/3rd-party)

add_executable(mc src/mc.cpp)
cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.16)

project(RayTracingWeekend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_subdirectory("${CMAKE_SOURCE_DIR}/test")

include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG master
)
FetchContent_MakeAvailable(fmt)

find_package(Threads REQUIRED)
#find_package(nlohmann_json REQUIRED)

add_executable(main src/main.cpp)
target_link_libraries(main Threads::Threads nlohmann_json::nlohmann_json fmt::fmt)
target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/3rd-party)

add_executable(mc src/mc.cpp)
target_link_libraries(mc nlohmann_json::nlohmann_json)
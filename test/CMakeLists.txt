include(FetchContent)
FetchContent_Declare(google_test
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG HEAD
)
FetchContent_MakeAvailable(google_test)

include_directories(${CMAKE_SOURCE_DIR}/src)
include(GoogleTest)

add_executable(vec3 vec3.cpp)
target_link_libraries(vec3 gtest_main)
gtest_discover_tests(vec3)

add_executable(color color.cpp)
target_link_libraries(color gtest_main)
gtest_discover_tests(color)

add_executable(ray ray.cpp)
target_link_libraries(ray gtest_main)
gtest_discover_tests(ray)

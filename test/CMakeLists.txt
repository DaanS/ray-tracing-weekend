include(FetchContent)
FetchContent_Declare(google_test
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG HEAD
)
FetchContent_MakeAvailable(google_test)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rd-party
)
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

add_executable(sphere sphere.cpp)
target_link_libraries(sphere gtest_main)
gtest_discover_tests(sphere)

add_executable(hittable_list hittable_list.cpp)
target_link_libraries(hittable_list gtest_main)
gtest_discover_tests(hittable_list)

add_executable(camera camera.cpp)
target_link_libraries(camera gtest_main)
gtest_discover_tests(camera)

add_executable(canvas canvas.cpp)
target_link_libraries(canvas gtest_main)
gtest_discover_tests(canvas)

add_executable(material material.cpp)
target_link_libraries(material gtest_main)
gtest_discover_tests(material)

add_executable(aabb aabb.cpp)
target_link_libraries(aabb gtest_main)
gtest_discover_tests(aabb)

add_executable(bvh bvh.cpp)
target_link_libraries(bvh gtest_main)
gtest_discover_tests(bvh)

add_executable(texture texture.cpp)
target_link_libraries(texture gtest_main)
gtest_discover_tests(texture)

add_executable(json json.cpp)
target_link_libraries(json gtest_main)
gtest_discover_tests(json)
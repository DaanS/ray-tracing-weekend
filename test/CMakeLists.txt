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

add_library(test_libs INTERFACE)
target_link_libraries(test_libs INTERFACE gtest_main nlohmann_json::nlohmann_json fmt::fmt)

add_executable(vec3 vec3.cpp)
target_link_libraries(vec3 test_libs)
gtest_discover_tests(vec3)

add_executable(ray ray.cpp)
target_link_libraries(ray test_libs)
gtest_discover_tests(ray)

add_executable(sphere sphere.cpp)
target_link_libraries(sphere test_libs)
gtest_discover_tests(sphere)

add_executable(hittable_list hittable_list.cpp)
target_link_libraries(hittable_list test_libs)
gtest_discover_tests(hittable_list)

add_executable(camera camera.cpp)
target_link_libraries(camera test_libs)
gtest_discover_tests(camera)

add_executable(canvas canvas.cpp)
target_link_libraries(canvas test_libs)
gtest_discover_tests(canvas)

add_executable(material material.cpp)
target_link_libraries(material test_libs)
gtest_discover_tests(material)

add_executable(aabb aabb.cpp)
target_link_libraries(aabb test_libs)
gtest_discover_tests(aabb)

add_executable(bvh bvh.cpp)
target_link_libraries(bvh test_libs)
gtest_discover_tests(bvh)

add_executable(texture texture.cpp)
target_link_libraries(texture test_libs)
gtest_discover_tests(texture)

add_executable(json json.cpp)
target_link_libraries(json test_libs)
gtest_discover_tests(json)

add_executable(scene scene.cpp)
target_link_libraries(scene test_libs)
gtest_discover_tests(scene)

add_executable(onb onb.cpp)
target_link_libraries(onb test_libs)
gtest_discover_tests(onb)

add_executable(spectrum spectrum.cpp)
target_link_libraries(spectrum test_libs)
gtest_discover_tests(spectrum)

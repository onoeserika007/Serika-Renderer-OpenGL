cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE SERIKA_UNIT_TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(serika_unit_tests ${SERIKA_UNIT_TEST_SOURCES})

target_link_libraries(serika_unit_tests PRIVATE
    renderer_lib
    glad_lib
    glfw
    GTest::gtest_main
)

add_test(NAME serika_unit_tests COMMAND serika_unit_tests)

set(SERIKA_TEST_JOBS 1 CACHE STRING "Parallel test job count")

add_custom_target(build-unit-tests
    DEPENDS serika_unit_tests
)

add_custom_target(check-unit-tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${SERIKA_TEST_JOBS}
    DEPENDS serika_unit_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

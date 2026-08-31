set(CMAKE_FOLDER "tests")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

add_executable(
  misc_tests
  misc_tests.cpp
)
target_link_libraries(
  misc_tests
  GTest::gtest_main
  misc
)
gtest_discover_tests(misc_tests)

# String utility tests against the real pstring implementation
add_executable(
  misc_util_tests
  misc_util_tests.cpp
)
target_link_libraries(
  misc_util_tests
  GTest::gtest_main
  misc
)
gtest_discover_tests(misc_util_tests)

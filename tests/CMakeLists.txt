include(CTest)
include(GoogleTest)
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
)

FetchContent_MakeAvailable(googletest)

add_executable(Tests ${CMAKE_SOURCE_DIR}/tests/tests.cpp)

target_link_libraries(Tests gtest_main)

gtest_discover_tests(Tests)
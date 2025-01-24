cmake_minimum_required(VERSION 3.5.0)
project(jeaux VERSION 0.1.0 LANGUAGES C CXX)

find_library(LIBBT libbt.so REQUIRED)
find_library(LIBJUTTA libjutta_bt_proto.so REQUIRED)
find_library(LIBMARIA libmariadbcpp.so REQUIRED)
find_package(fmt REQUIRED)

add_executable(jeaux src/main.cpp)

target_link_libraries(jeaux ${LIBBT} ${LIBJUTTA} ${LIBMARIA} fmt::fmt)

include(CTest)
enable_testing()

file(COPY ${CMAKE_SOURCE_DIR}/resources/machinefiles DESTINATION ${CMAKE_BINARY_DIR})
cmake_minimum_required(VERSION 3.5.0)
project(jeaux VERSION 0.1.0 LANGUAGES C CXX)

#find_library(LIBBT libbt.so REQUIRED)
#find_library(LIBJUTTA libjutta_bt_proto.so REQUIRED)
find_library(LIBMARIA libmariadbcpp.so REQUIRED)
find_package(fmt REQUIRED)

add_executable(jeaux src/main.cpp)

target_include_directories(jeaux PUBLIC 
                            ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/src/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/_deps/eventpp-src/include)

target_link_libraries(jeaux
                        ${LIBMARIA}
                        fmt::fmt
                        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/src/bt/libbt.so
                        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/src/jutta_bt_proto/libjutta_bt_proto.so)

file(COPY ${CMAKE_SOURCE_DIR}/resources/machinefiles DESTINATION ${CMAKE_BINARY_DIR})
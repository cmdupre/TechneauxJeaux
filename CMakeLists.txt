set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
cmake_minimum_required(VERSION 3.28.3)
project(jeaux VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_library(LIBBT libbt.so REQUIRED)
#find_library(LIBJUTTA libjutta_bt_proto.so REQUIRED)
find_library(LIBMARIA libmariadbcpp.so REQUIRED)
find_package(fmt REQUIRED)

add_executable(jeaux src/main.cpp)

target_include_directories(jeaux PUBLIC 
                            ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/src/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/_deps/eventpp-src/include)

target_link_libraries(jeaux
                        fmt::fmt
                        ${LIBMARIA}
                        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/src/bt/libbt.so
                        ${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/src/jutta_bt_proto/libjutta_bt_proto.so)

set_target_properties(jeaux PROPERTIES
    INSTALL_RPATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/src/bt;${CMAKE_CURRENT_SOURCE_DIR}/../protocol-bt-cpp/build/src/jutta_bt_proto"
)

add_subdirectory(tests)
enable_testing()

# Use install instead.
#file(COPY ${CMAKE_SOURCE_DIR}/resources/machinefiles DESTINATION ${CMAKE_BINARY_DIR})
install(TARGETS jeaux DESTINATION .)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/machinefiles DESTINATION .)
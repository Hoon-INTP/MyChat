cmake_minimum_required(VERSION 3.10)

project(ServerApp)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.70 REQUIRED COMPONENTS system)

add_executable(server server.cpp)
target_link_libraries(server Boost::system)
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

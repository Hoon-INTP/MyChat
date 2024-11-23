cmake_minimum_required(VERSION 3.10)

project(ClientApp)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.70 REQUIRED COMPONENTS system)

add_executable(client client.cpp)
target_link_libraries(client Boost::system)
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

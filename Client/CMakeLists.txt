cmake_minimum_required(VERSION 3.0)
project(Client)
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/../TcpSocket)
file(GLOB SRC_LIST_1 ${CMAKE_CURRENT_SOURCE_DIR}/../TcpSocket/*.cpp)
file(GLOB SRC_LIST_2 ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list(APPEND SRC_LIST_1 ${SRC_LIST_1} ${SRC_LIST_2} ${TEMP})

add_executable(Client ${SRC_LIST_1})

# 输出源文件列表
message(STATUS "Source files: ${SRC_LIST_1}")
cmake_minimum_required(VERSION 3.0)
project(Server)
set(CMAKE_CXX_STANDARD 11)

include_directories(${PROJECT_SOURCE_DIR}/../TcpSocket)
include_directories(${PROJECT_SOURCE_DIR}/../OS)
file(GLOB SRC_LIST_1 ${CMAKE_CURRENT_SOURCE_DIR}/../TcpSocket/*.cpp)
file(GLOB SRC_LIST_2 ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB SRC_LIST_3 ${CMAKE_CURRENT_SOURCE_DIR}/../OS/OperatingSystem.cpp)
file(GLOB SRC_LIST_4 ${CMAKE_CURRENT_SOURCE_DIR}/../OS/FileSnapshot.cpp)
file(GLOB SRC_LIST_5 ${CMAKE_CURRENT_SOURCE_DIR}/../OS/Log.cpp)

list(APPEND SRC_LIST_1 ${SRC_LIST_1} ${SRC_LIST_2} ${TEMP})
list(APPEND SRC_LIST_1 ${SRC_LIST_1} ${SRC_LIST_3} ${TEMP})
list(APPEND SRC_LIST_1 ${SRC_LIST_1} ${SRC_LIST_4} ${TEMP})
list(APPEND SRC_LIST_1 ${SRC_LIST_1} ${SRC_LIST_5} ${TEMP})

add_executable(Server ${SRC_LIST_1})

# 输出源文件列表
message(STATUS "Source files: ${SRC_LIST_1}")
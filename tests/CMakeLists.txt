cmake_minimum_required(VERSION 3.10)
project(json5_test)

set(SRC vistor.cpp)

include_directories(${PROJECT_SOURCE_DIR}/../include)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} "json5_parser")

cmake_minimum_required(VERSION 3.10)
project(json5_test)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-std=c++14 -stdlib=libc++")
	set(CMAKE_C_FLAGS "-std=c11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "-std=c++14")
	set(CMAKE_C_FLAGS "-std=c11")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "/std:c++14 /EHsc")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "-std=c++14")
	set(CMAKE_C_FLAGS "-std=c11")
endif()

set(SRC vistor.cpp)

include_directories(${PROJECT_SOURCE_DIR}/../include)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} "json5_parser")

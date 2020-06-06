cmake_minimum_required(VERSION 3.10)
project(json5_parser)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-std=c++14 -stdlib=libc++ -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-std=c11 -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "-std=c++14 -stdlib=libc++ -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-std=c11 -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "/std:c++14 /EHa")
	set(CMAKE_CXX_FLAGS_RELEASE "/MD /Ox /Ob2 /Oi /Ot /Oy /GL /GF /GS- /Gy /Gd /Zi /fp:fast /Zc:inline")
	set(CMAKE_STATIC_LINKER_FLAGS__RELEASE "/LTCG /NXCOMPAT /OPT:REF /OPT:ICF")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "-std=c++14 -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-std=c11 -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

option(OPT_ENABLE_JSON5 "Enable json5" ON)
if(OPT_ENABLE_JSON5)
	set(JSON_ENABLE_JSON5 1)
	set(JSON5_ENABLE_COMMENTS 1)
endif()

configure_file(
${PROJECT_SOURCE_DIR}/config.h.in
${PROJECT_SOURCE_DIR}/include/json5_parser/config.h)


include_directories(
${PROJECT_SOURCE_DIR}/include
)

set(SRC 
"src/json5_parser.cpp"
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(LIB_NAME json5_parser)
add_library(${LIB_NAME} STATIC ${SRC})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/json5_parser DESTINATION "include")
install(TARGETS ${LIB_NAME} ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_subdirectory(./benchmark)
add_subdirectory(./test)

cmake_minimum_required(VERSION 3.10)
project(json5_parser)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "-std=c++1z -stdlib=libc++ -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-std=c11 -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
	set(CMAKE_CXX_FLAGS "-std=c++1z -stdlib=libc++ -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-std=c11 -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "/std:c++17 /EHa")
	set(CMAKE_CXX_FLAGS_RELEASE "/MD /Ox /Ob2 /Oi /Ot /Oy /GF /GS- /Gy /Gd /Zi /fp:fast /Zc:inline")
	set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "-std=c++1z -fvisibility=hidden")
	set(CMAKE_C_FLAGS "-std=c11 -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

option(OPT_ENABLE_JSON5 "Enable json5" ON)
if(OPT_ENABLE_JSON5)
	set(JSON_ENABLE_JSON5 1)
	set(JSON5_ENABLE_COMMENTS 1)
endif()
option(OPT_ENABLE_SIMD_PARSER "Enable 'simd_double_parser'" ON)
if(OPT_ENABLE_SIMD_PARSER)
	set(JSON_ENABLE_SIMD_PARSER 1)
endif()
option(OPT_ENABLE_DEBUG_WALKER "Enable debug walker" OFF)
if(OPT_ENABLE_DEBUG_WALKER)
	set(JSON_ENABLE_DEBUG_WALKER 1)
endif()
option(OPT_ENABLE_DOM_WALKER "Enable default DOM walker" ON)
if(OPT_ENABLE_DOM_WALKER)
	set(JSON_ENABLE_DOM_WALKER 1)
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

cmake_minimum_required(VERSION 3.10)
project(json5_parser VERSION 1.0)

add_definitions("-DJSON5_DYNAMIC_LIBRARY_EXPORTS=1")

if(WIN32)
	set(CMAKE_CXX_FLAGS_MINSIZEREL "/MD /W3 /WX /GF /GS- /Gy /Gd /Ox /Ob2 /Oy /Oi /Os /EHsc /Zi /fp:fast /Zc:inline")
	set(CMAKE_CXX_FLAGS_RELEASE    "/MD /W3 /WX /GF /GS- /Gy /Gd /Ox /Ob2 /Oy /Oi /Os /EHsc /Zi /fp:fast /Zc:inline")
	set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
endif()


option(JSON5_ENABLE_JSON5 "Enable json5" ON)
if(JSON5_ENABLE_JSON5)
	set(JSON_ENABLE_JSON5 1)
	set(JSON5_ENABLE_COMMENTS 1)
endif()

option(JSON5_ENABLE_SIMD_PARSER "Enable 'simd_double_parser'" OFF)
if(JSON5_ENABLE_SIMD_PARSER)
	set(JSON_ENABLE_SIMD_PARSER 1)
endif()

option(JSON5_ENABLE_DEBUG_HANDLER "Enable debug SAX handler" OFF)
if(JSON5_ENABLE_DEBUG_HANDLER)
	set(JSON_ENABLE_DEBUG_HANDLER 1)
endif()

option(JSON5_ENABLE_RAPID_HANDLER "Enable default DOM handler" ON)
if(JSON5_ENABLE_RAPID_HANDLER)
	set(JSON_ENABLE_RAPID_HANDLER 1)
endif()

option(JSON5_ENABLE_GRISU3 "Enable default DOM handler" ON)
if(JSON5_ENABLE_GRISU3)
	set(JSON_ENABLE_GRISU3 1)
endif()

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
${CMAKE_CURRENT_SOURCE_DIR}/include/json5_parser/config.h)

set(SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/json5_parser/json5_parser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/json5/json5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/json5/grisu3.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include
	${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json/include
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE UTF)
if(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Acl.${PROJECT_NAME}")
else()
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "libAcl.${PROJECT_NAME}")
endif()

# ----------------------------------- tests -----------------------------------
if(CMAKE_ENABLE_UNIT_TEST)
	set(TESTS_SIMD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_simd_parser.cpp)
	add_executable(testJson5_Simd ${TESTS_SIMD_SRC})
	target_link_libraries(testJson5_Simd PUBLIC ${PROJECT_NAME})
	target_include_directories(testJson5_Simd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

	set(TESTS_VISTOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests/vistor.cpp)
	add_executable(testJson5_Vistor ${TESTS_VISTOR_SRC})
	target_link_libraries(testJson5_Vistor PUBLIC ${PROJECT_NAME})
	target_include_directories(testJson5_Vistor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

	set(TESTS_WVISTOR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests/wvistor.cpp)
	add_executable(testJson5_WVisitor ${TESTS_WVISTOR_SRC})
	target_link_libraries(testJson5_WVisitor PUBLIC ${PROJECT_NAME})
	target_include_directories(testJson5_WVisitor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

	set(TESTS_JSON5_SRC ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_json_v2.cpp)
	add_executable(testJson5 ${TESTS_JSON5_SRC})
	target_link_libraries(testJson5 PUBLIC ${PROJECT_NAME})
	target_include_directories(testJson5 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# ---------------------------------- benchmark ----------------------------------
if(CMAKE_ENABLE_UNIT_TEST)
	set(TESTS_BENCHMARK_SRC ${CMAKE_CURRENT_SOURCE_DIR}/benchmark/benchmark_parsr.cpp)
	add_executable(benchmarkJson5 ${TESTS_BENCHMARK_SRC})
	target_link_libraries(benchmarkJson5 PUBLIC ${PROJECT_NAME})
	target_include_directories(benchmarkJson5 PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include
		)
endif()

# ------------------------------- install script ------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/install.cmake)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/json5_parser DESTINATION include)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/json5 DESTINATION include)

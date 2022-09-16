#pragma once

#ifndef JSON5_STATIC_LIBRARY
#  if _WIN32
#    ifdef JSON5_DYNAMIC_LIBRARY_EXPORTS
#      define JSON5_API __declspec(dllexport)
#    else //JSON5_DYNAMIC_LIBRARY_EXPORTS
#      define JSON5_API __declspec(dllimport)
#    endif //JSON5_DYNAMIC_LIBRARY_EXPORTS
#  else //_WIN32
#    define JSON5_API __attribute__((visibility("default")))
#  endif //_WIN32
#else //JSON5_STATIC_LIBRARY
#  define JSON5_API
#endif //JSON5_STATIC_LIBRARY

#ifndef JSON_ENABLE_JSON5
#define JSON_ENABLE_JSON5 1
#endif	//JSON_ENABLE_JSON5

#if JSON_ENABLE_JSON5
#define JSON5_ENABLE_COMMENTS	1
#else
#endif	//JSON_ENABLE_JSON5

/* #undef JSON_ENABLE_DEBUG_HANDLER */
#define JSON_ENABLE_RAPID_HANDLER 1
/* #undef JSON_ENABLE_SIMD_PARSER */
#define JSON_ENABLE_GRISU3 1

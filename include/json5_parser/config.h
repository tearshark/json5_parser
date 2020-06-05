#pragma once

#ifndef JSON_ENABLE_JSON5
/* #undef JSON_ENABLE_JSON5 */
#endif	//JSON_ENABLE_JSON5

#if JSON_ENABLE_JSON5
#define JSON5_IF_ENABLE(x)		x
#define JSON5_ENABLE_COMMENTS	
#else
#define JSON5_IF_ENABLE(x)
#endif	//JSON_ENABLE_JSON5

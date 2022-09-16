#include <inttypes.h>
#include <cassert>
#include <ctype.h>
#include <string>
#include <string_view>
#include <algorithm>
#include <functional>
#include <cstring>
#include <new>
#include <cmath>
#include <memory>
#include <string.h>

#if _MSC_VER
#pragma warning(disable : 26819)    //warning C26819: Unannotated fallthrough between switch labels.
#endif

#include "json5_parser/const_def.h"
#include "json5_parser/config.h"
#if JSON_ENABLE_SIMD_PARSER
#include "simd_double_parser.h"     //���ۺϲ����У���5% ~ 30%�����������������Ⱦ���strtod��һ�¡���16λ�����ϲ�һ�¡�
#else
#include "fast_double_parser.h"
#endif

namespace json5
{
#include "json5_parser/xchar/xchar.h"
#include "json5_parser_common.hpp"

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/uchar_def.h"
#undef JSON_U
#define JSON_U(c,r)	if((c)>255) return (r)

    namespace unicode
    {
#include "json5_parser/sax_handler.inl"
#include "json5_parser/parser.inl"
#include "parser.hpp"

#if JSON_ENABLE_RAPID_HANDLER
#include "json5_parser/rapid_dom_handler.inl"
#include "rapid_dom_handler.hpp"
#endif
    }

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
#undef JSON_U
#define JSON_U(c,r)

    namespace singlebyte
    {
#include "json5_parser/sax_handler.inl"
#include "json5_parser/parser.inl"
#include "parser.hpp"

#if JSON_ENABLE_RAPID_HANDLER
#include "json5_parser/rapid_dom_handler.inl"
#include "rapid_dom_handler.hpp"
#endif
    }

}
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

#include "json5_parser/const_def.h"
#include "json5_parser/config.h"

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
#include "json5_parser/walker.inl"
#include "json5_parser/parser.inl"
#include "parser.hpp"

#if JSON_ENABLE_DOM_WALKER
#include "json5_parser/dom_walker.inl"
#include "dom_walker.hpp"
#endif
}

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
#undef JSON_U
#define JSON_U(c,r)

namespace singlebyte
{
#include "json5_parser/walker.inl"
#include "json5_parser/parser.inl"
#include "parser.hpp"

#if JSON_ENABLE_DOM_WALKER
#include "json5_parser/dom_walker.inl"
#include "dom_walker.hpp"
#endif
}

}
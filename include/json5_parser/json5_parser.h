#pragma once

#include <stdlib.h>
#include <string>
#include <iostream>
#include <string_view>
#include <functional>

#include "json5_parser/config.h"

namespace json5
{
#include "json5_parser/const_def.h"
#include "json5_parser/xchar/xchar.h"

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/uchar_def.h"
    namespace unicode
    {
#include "json5_parser/sax_handler.inl"
#include "json5_parser/dummy_handler.inl"
#include "json5_parser/parser.inl"

#if JSON_ENABLE_DEBUG_HANDLER
#include "json5_parser/debug_handler.inl"
#endif
#if JSON_ENABLE_RAPID_HANDLER
#include "json5_parser/rapid_dom_handler.inl"
#endif
    }

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
    namespace singlebyte
    {
#include "json5_parser/sax_handler.inl"
#include "json5_parser/dummy_handler.inl"
#include "json5_parser/parser.inl"

#if JSON_ENABLE_DEBUG_HANDLER
#include "json5_parser/debug_handler.inl"
#endif
#if JSON_ENABLE_RAPID_HANDLER
#include "json5_parser/rapid_dom_handler.inl"
#endif
    }

    using js_parser = singlebyte::js_parser;
    using wjs_parser = unicode::js_parser;

#if JSON_ENABLE_RAPID_HANDLER
    using rapid_value = singlebyte::rapid_value;
    using rapid_dom_handler = singlebyte::rapid_dom_handler;

    using rapid_wvalue = unicode::rapid_value;
    using rapid_wdom_handler = unicode::rapid_dom_handler;
#endif
}

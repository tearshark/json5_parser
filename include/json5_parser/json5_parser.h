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
#include "json5_parser/parser.inl"
		typedef JSON_Parser parser;

#if JSON_ENABLE_DEBUG_HANDLER
#include "json5_parser/debug_handler.inl"
#endif
#if JSON_ENABLE_DOM_HANDLER
#include "json5_parser/dom_handler.inl"
		typedef JSON_Value value;
		typedef SAX_DOMHandler dom_handler;
#endif
	}

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
	namespace singlebyte
	{
#include "json5_parser/sax_handler.inl"
#include "json5_parser/parser.inl"
		typedef JSON_Parser parser;

#if JSON_ENABLE_DEBUG_HANDLER
#include "json5_parser/debug_handler.inl"
#endif
#if JSON_ENABLE_DOM_HANDLER
#include "json5_parser/dom_handler.inl"
		typedef JSON_Value value;
		typedef SAX_DOMHandler dom_handler;
#endif
	}

	typedef singlebyte::parser parser;
	typedef unicode::parser wparser;

#if JSON_ENABLE_DOM_HANDLER
	typedef singlebyte::value value;
	typedef singlebyte::dom_handler dom_handler;
	typedef unicode::value wvalue;
	typedef unicode::dom_handler wdom_handler;
#endif
}

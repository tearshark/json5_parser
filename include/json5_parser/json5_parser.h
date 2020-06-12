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
#include "json5_parser/walker.inl"
#include "json5_parser/parser.inl"
		typedef JSON_Parser parser;

#if JSON_ENABLE_DEBUG_WALKER
#include "json5_parser/debug_walker.inl"
#endif
#if JSON_ENABLE_DOM_WALKER
#include "json5_parser/dom_walker.inl"
		typedef JSON_Value value;
		typedef JSON_DOMWalker walker;
#endif
	}

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
	namespace singlebyte
	{
#include "json5_parser/walker.inl"
#include "json5_parser/parser.inl"
		typedef JSON_Parser parser;

#if JSON_ENABLE_DEBUG_WALKER
#include "json5_parser/debug_walker.inl"
#endif
#if JSON_ENABLE_DOM_WALKER
#include "json5_parser/dom_walker.inl"
		typedef JSON_Value value;
		typedef JSON_DOMWalker walker;
#endif
	}

	typedef singlebyte::parser parser;
	typedef unicode::parser wparser;

#if JSON_ENABLE_DOM_WALKER
	typedef singlebyte::value value;
	typedef singlebyte::walker walker;
	typedef unicode::value wvalue;
	typedef unicode::walker wwalker;
#endif
}

#pragma once

#include <stdlib.h>
#include <string>
#include <iostream>
#include <string_view>
#include <functional>

#include "json5_parser/const_def.h"

namespace json5
{
#include "json5_parser/xchar/xchar.h"

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/uchar_def.h"
	namespace unicode
	{
#include "json5_parser/walker.inl"
#include "json5_parser/parser.inl"
//#include "json5_parser/debug_walker.inl"
#include "json5_parser/dom_walker.inl"

		typedef JSON_Parser parser;
		typedef JSON_Value value;
		typedef JSON_DOMWalker walker;
	}

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
	namespace singlebyte
	{
#include "json5_parser/walker.inl"
#include "json5_parser/parser.inl"
//#include "json5_parser/debug_walker.inl"
#include "json5_parser/dom_walker.inl"

		typedef JSON_Parser parser;
		typedef JSON_Value value;
		typedef JSON_DOMWalker walker;
	}

	typedef singlebyte::parser parser;
	typedef singlebyte::value value;
	typedef singlebyte::walker walker;

	typedef unicode::parser wparser;
	typedef unicode::value wvalue;
	typedef unicode::walker wwalker;
}

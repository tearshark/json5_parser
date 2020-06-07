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
#include "json5_parser/debug_walker.inl"
#include "json5_parser/dom_walker.inl"
	}

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
	namespace singlebyte
	{
#include "json5_parser/walker.inl"
#include "json5_parser/parser.inl"
#include "json5_parser/debug_walker.inl"
#include "json5_parser/dom_walker.inl"
	}

	typedef singlebyte::JSON_Parser parser;
	typedef unicode::JSON_Parser wparser;

	typedef singlebyte::JSON_Value value;
	typedef unicode::JSON_Value wvalue;
}

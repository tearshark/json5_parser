#pragma once
#include <stdlib.h>
#include <string>

#include "json5_parser/const_def.h"

namespace json5
{
#include "json5_parser/xchar/xchar.h"

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/uchar_def.h"
	namespace unicode
	{
#include "json5_parser/parser.hpp"
	}

#include "json5_parser/xchar/xchar_undef.h"
#include "json5_parser/xchar/schar_def.h"
	namespace singlebyte
	{
#include "json5_parser/parser.hpp"
	}

	typedef singlebyte::JSON_Parser parser;
	typedef unicode::JSON_Parser wparser;

	typedef singlebyte::JSON_Value value;
	typedef unicode::JSON_Value wvalue;
}

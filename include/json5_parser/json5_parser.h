#pragma once
#include <stdlib.h>
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

/*
	inline namespace literals
	{
		inline json operator "" _js(const char * psz, size_t cnt)
		{
#if defined(UNICODE) || defined(_UNICODE)
			VString str(psz, cnt);
			return jobject::fromString(str, str.GetLength());
#else
			return jobject::fromString(psz, cnt);
#endif
		}
		inline json operator "" _js(const wchar_t * psz, size_t cnt)
		{
#if defined(UNICODE) || defined(_UNICODE)
			return jobject::fromString(psz, cnt);
#else
			VString str(psz, cnt);
			return jobject::fromString(str, str.GetLength());
#endif
		}

		inline ajson operator "" _ajs(const char * psz, size_t cnt)
		{
			return ajobject::fromString(psz, cnt);
		}
		inline ajson operator "" _ajs(const wchar_t * psz, size_t cnt)
		{
			VStringA str(psz, cnt);
			return ajobject::fromString(str, str.GetLength());
		}

		inline wjson operator "" _wjs(const char * psz, size_t cnt)
		{
			VStringW str(psz, cnt);
			return wjobject::fromString(str, str.GetLength());
		}
		inline wjson operator "" _wjs(const wchar_t * psz, size_t cnt)
		{
			return wjobject::fromString(psz, cnt);
		}
	}
*/
}

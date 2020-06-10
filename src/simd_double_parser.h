#pragma once
#include <limits>
#include <tuple>

#include <emmintrin.h>		//SSE2

namespace simd_double_parser
{
	union number_value
	{
		int64_t l;
		double d;
	};

	enum struct parser_result
	{
		Invalid,
		Long,
		Double,
	};

#include "simd_double_parser.inl"

	template<class _CharType>
	std::tuple<number_value, parser_result> parser(const _CharType*& psz, const _CharType* const pszEnd) noexcept
	{
		typedef x_convert_char_selector<sizeof(_CharType)> char_selector;
		typedef typename char_selector::type rechar_type;
		return simd_double_parser2((const rechar_type*&)psz, (const rechar_type*)pszEnd);
	}
}

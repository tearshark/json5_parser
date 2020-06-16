#pragma once
#include <limits>
#include <tuple>

#include <emmintrin.h>		//SSE2

namespace simd_double_parser
{
#ifdef _MSC_VER
#ifndef really_inline
#define really_inline __forceinline
#endif // really_inline
#ifndef unlikely
#define unlikely(x) x
#endif // unlikely
#ifndef disable_inline
#define disable_inline __declspec(noinline)
#endif
#else  // _MSC_VER
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif // unlikely
#ifndef really_inline
#define really_inline __attribute__((always_inline)) inline
#endif // really_inline
#ifndef disable_inline
#define disable_inline __attribute__((noinline))
#endif
#endif // _MSC_VER

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
	really_inline
	std::tuple<number_value, parser_result> parser(const _CharType*& psz, const _CharType* const pszEnd) noexcept
	{
		typedef x_convert_char_selector<sizeof(_CharType)> char_selector;
		typedef typename char_selector::type rechar_type;
		return simd_double_parser2((const rechar_type*&)psz, (const rechar_type*)pszEnd);
	}
}

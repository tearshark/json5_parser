#pragma once
#include <cstdint>

#define VFX_API

#ifndef _MSC_VER
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#define _aligned_free(pointer) free(pointer)
#endif

namespace json5
{
	enum JSON_Type : uint8_t
	{
		JSONT_Nullptr,
		JSONT_Object,
		JSONT_Array,
		JSONT_String,
		JSONT_Double,
		JSONT_Long,
		JSONT_Boolean,

		JSONT_LONG_MASK = 0x0F,
		JSONT_BinaryLong = JSONT_Long + 0x10,
		JSONT_OctalLong = JSONT_Long + 0x70,
		JSONT_DecimalLong = JSONT_Long + 0x90,
		JSONT_HexLong = JSONT_Long + 0xF0,
	};

	inline JSON_Type operator & (JSON_Type left, JSON_Type right)
	{
		return (JSON_Type)((uint8_t)left & (uint8_t)right);
	}
	inline JSON_Type operator | (JSON_Type left, JSON_Type right)
	{
		return (JSON_Type)((uint8_t)left | (uint8_t)right);
	}

	const uint32_t JSONT_NOT_CLASS = 0x78;

	struct JSON_Value;
}

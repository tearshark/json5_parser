
#if JSON_ENABLE_JSON5
#define JSON5_IF_ENABLE(x)		x
#else
#define JSON5_IF_ENABLE(x)
#endif	//JSON_ENABLE_JSON5

#if !defined(_WIN32) && !defined(_WIN64)
#ifndef _aligned_malloc
#define _aligned_malloc(size, alignment) aligned_alloc(alignment, size)
#endif
#ifndef _aligned_free
#define _aligned_free(pointer) free(pointer)
#endif
#endif

#if __GNUC__
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

enum struct JSON_Type : uint8_t
{
    Long,
    Double,
    String,
    Boolean,
    Object,
    Array,
    Nullptr,

    LONG_MASK = 0x0F,
    BinaryLong = Long + 0x10,
    OctalLong = Long + 0x70,
    DecimalLong = Long + 0x90,
    HexLong = Long + 0xF0,
};

inline JSON_Type operator & (JSON_Type left, JSON_Type right)
{
    return (JSON_Type)((uint8_t)left & (uint8_t)right);
}
inline JSON_Type operator | (JSON_Type left, JSON_Type right)
{
    return (JSON_Type)((uint8_t)left | (uint8_t)right);
}

const uint32_t JSONT_NOT_CLASS = 0xCF;

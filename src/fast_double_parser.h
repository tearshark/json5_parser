
#ifndef FAST_DOUBLE_PARSER_H
#define FAST_DOUBLE_PARSER_H

#include <cfloat>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <type_traits>
#include <wchar.h>

#ifdef _MSC_VER
#include <intrin.h>
#define WARN_UNUSED
#else
#define WARN_UNUSED __attribute__((warn_unused_result))
#endif

namespace fast_double_parser {

    enum struct result_type
    {
        Invalid,
        Long,
        Double,
    };

#define FASTFLOAT_SMALLEST_POWER -325
#define FASTFLOAT_LARGEST_POWER 308

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

    struct value128 {
        uint64_t low;
        uint64_t high;
    };

#ifdef _MSC_VER
#define FAST_DOUBLE_PARSER_VISUAL_STUDIO 1
#ifdef __clang__
    // clang under visual studio
#define FAST_DOUBLE_PARSER_CLANG_VISUAL_STUDIO 1
#else
    // just regular visual studio (best guess)
#define FAST_DOUBLE_PARSER_REGULAR_VISUAL_STUDIO 1
#endif // __clang__
#endif // _MSC_VER

#if defined(FAST_DOUBLE_PARSER_REGULAR_VISUAL_STUDIO) &&                                 \
    !defined(_M_X64) && !defined(_M_ARM64)// _umul128 for x86, arm
// this is a slow emulation routine for 32-bit Windows
//
    static really_inline uint64_t __emulu(uint32_t x, uint32_t y) {
        return x * (uint64_t)y;
    }
    static really_inline uint64_t _umul128(uint64_t ab, uint64_t cd, uint64_t* hi) {
        uint64_t ad = __emulu((uint32_t)(ab >> 32), (uint32_t)cd);
        uint64_t bd = __emulu((uint32_t)ab, (uint32_t)cd);
        uint64_t adbc = ad + __emulu((uint32_t)ab, (uint32_t)(cd >> 32));
        uint64_t adbc_carry = !!(adbc < ad);
        uint64_t lo = bd + (adbc << 32);
        *hi = __emulu((uint32_t)(ab >> 32), (uint32_t)(cd >> 32)) + (adbc >> 32) +
            (adbc_carry << 32) + !!(lo < bd);
        return lo;
    }
#endif

    really_inline value128 full_multiplication(uint64_t value1, uint64_t value2) {
        value128 answer;
#ifdef FAST_DOUBLE_PARSER_REGULAR_VISUAL_STUDIO
#ifdef _M_ARM64
        // ARM64 has native support for 64-bit multiplications, no need to emultate
        answer.high = __umulh(value1, value2);
        answer.low = value1 * value2;
#else
        answer.low = _umul128(value1, value2, &answer.high); // _umul128 not available on ARM64
#endif // _M_ARM64
#else // SIMDJSON_REGULAR_VISUAL_STUDIO
        __uint128_t r = ((__uint128_t)value1) * value2;
        answer.low = uint64_t(r);
        answer.high = uint64_t(r >> 64);
#endif
        return answer;
    }


    /* result might be undefined when input_num is zero */
    really_inline int leading_zeroes(uint64_t input_num) {
#ifdef _MSC_VER
        unsigned long leading_zero = 0;
        // Search the mask data from most significant bit (MSB)
        // to least significant bit (LSB) for a set bit (1).
        if (_BitScanReverse64(&leading_zero, input_num))
            return (int)(63 - leading_zero);
        else
            return 64;
#else
        return __builtin_clzll(input_num);
#endif // _MSC_VER
    }

    // Precomputed powers of ten from 10^0 to 10^22. These
    // can be represented exactly using the double type.
    static const double power_of_ten[] = {
        1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
        1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22 };

    template<class _Char>
    static really_inline bool is_integer(_Char c) {
        return (c >= (_Char)'0' && c <= (_Char)'9');
        // this gets compiled to (uint8_t)(c - '0') <= 9 on all decent compilers
    }

    // the mantissas of powers of ten from FASTFLOAT_SMALLEST_POWER to FASTFLOAT_LARGEST_POWER, extended out to sixty four
    // bits
    // This struct will likely get padded to 16 bytes.
    typedef struct {
        uint64_t mantissa;
        int32_t exp;
    } components;

    // The array power_of_ten_components contain the powers of ten approximated
    // as a 64-bit mantissa, with an exponent part. It goes from 10^
    // FASTFLOAT_SMALLEST_POWER to
    // 10^FASTFLOAT_LARGEST_POWER (inclusively). The mantissa is truncated, and
    // never rounded up.
    // Uses about 10KB.
    static const components power_of_ten_components[] = {
        {0xa5ced43b7e3e9188L, 7},    {0xcf42894a5dce35eaL, 10},
        {0x818995ce7aa0e1b2L, 14},   {0xa1ebfb4219491a1fL, 17},
        {0xca66fa129f9b60a6L, 20},   {0xfd00b897478238d0L, 23},
        {0x9e20735e8cb16382L, 27},   {0xc5a890362fddbc62L, 30},
        {0xf712b443bbd52b7bL, 33},   {0x9a6bb0aa55653b2dL, 37},
        {0xc1069cd4eabe89f8L, 40},   {0xf148440a256e2c76L, 43},
        {0x96cd2a865764dbcaL, 47},   {0xbc807527ed3e12bcL, 50},
        {0xeba09271e88d976bL, 53},   {0x93445b8731587ea3L, 57},
        {0xb8157268fdae9e4cL, 60},   {0xe61acf033d1a45dfL, 63},
        {0x8fd0c16206306babL, 67},   {0xb3c4f1ba87bc8696L, 70},
        {0xe0b62e2929aba83cL, 73},   {0x8c71dcd9ba0b4925L, 77},
        {0xaf8e5410288e1b6fL, 80},   {0xdb71e91432b1a24aL, 83},
        {0x892731ac9faf056eL, 87},   {0xab70fe17c79ac6caL, 90},
        {0xd64d3d9db981787dL, 93},   {0x85f0468293f0eb4eL, 97},
        {0xa76c582338ed2621L, 100},  {0xd1476e2c07286faaL, 103},
        {0x82cca4db847945caL, 107},  {0xa37fce126597973cL, 110},
        {0xcc5fc196fefd7d0cL, 113},  {0xff77b1fcbebcdc4fL, 116},
        {0x9faacf3df73609b1L, 120},  {0xc795830d75038c1dL, 123},
        {0xf97ae3d0d2446f25L, 126},  {0x9becce62836ac577L, 130},
        {0xc2e801fb244576d5L, 133},  {0xf3a20279ed56d48aL, 136},
        {0x9845418c345644d6L, 140},  {0xbe5691ef416bd60cL, 143},
        {0xedec366b11c6cb8fL, 146},  {0x94b3a202eb1c3f39L, 150},
        {0xb9e08a83a5e34f07L, 153},  {0xe858ad248f5c22c9L, 156},
        {0x91376c36d99995beL, 160},  {0xb58547448ffffb2dL, 163},
        {0xe2e69915b3fff9f9L, 166},  {0x8dd01fad907ffc3bL, 170},
        {0xb1442798f49ffb4aL, 173},  {0xdd95317f31c7fa1dL, 176},
        {0x8a7d3eef7f1cfc52L, 180},  {0xad1c8eab5ee43b66L, 183},
        {0xd863b256369d4a40L, 186},  {0x873e4f75e2224e68L, 190},
        {0xa90de3535aaae202L, 193},  {0xd3515c2831559a83L, 196},
        {0x8412d9991ed58091L, 200},  {0xa5178fff668ae0b6L, 203},
        {0xce5d73ff402d98e3L, 206},  {0x80fa687f881c7f8eL, 210},
        {0xa139029f6a239f72L, 213},  {0xc987434744ac874eL, 216},
        {0xfbe9141915d7a922L, 219},  {0x9d71ac8fada6c9b5L, 223},
        {0xc4ce17b399107c22L, 226},  {0xf6019da07f549b2bL, 229},
        {0x99c102844f94e0fbL, 233},  {0xc0314325637a1939L, 236},
        {0xf03d93eebc589f88L, 239},  {0x96267c7535b763b5L, 243},
        {0xbbb01b9283253ca2L, 246},  {0xea9c227723ee8bcbL, 249},
        {0x92a1958a7675175fL, 253},  {0xb749faed14125d36L, 256},
        {0xe51c79a85916f484L, 259},  {0x8f31cc0937ae58d2L, 263},
        {0xb2fe3f0b8599ef07L, 266},  {0xdfbdcece67006ac9L, 269},
        {0x8bd6a141006042bdL, 273},  {0xaecc49914078536dL, 276},
        {0xda7f5bf590966848L, 279},  {0x888f99797a5e012dL, 283},
        {0xaab37fd7d8f58178L, 286},  {0xd5605fcdcf32e1d6L, 289},
        {0x855c3be0a17fcd26L, 293},  {0xa6b34ad8c9dfc06fL, 296},
        {0xd0601d8efc57b08bL, 299},  {0x823c12795db6ce57L, 303},
        {0xa2cb1717b52481edL, 306},  {0xcb7ddcdda26da268L, 309},
        {0xfe5d54150b090b02L, 312},  {0x9efa548d26e5a6e1L, 316},
        {0xc6b8e9b0709f109aL, 319},  {0xf867241c8cc6d4c0L, 322},
        {0x9b407691d7fc44f8L, 326},  {0xc21094364dfb5636L, 329},
        {0xf294b943e17a2bc4L, 332},  {0x979cf3ca6cec5b5aL, 336},
        {0xbd8430bd08277231L, 339},  {0xece53cec4a314ebdL, 342},
        {0x940f4613ae5ed136L, 346},  {0xb913179899f68584L, 349},
        {0xe757dd7ec07426e5L, 352},  {0x9096ea6f3848984fL, 356},
        {0xb4bca50b065abe63L, 359},  {0xe1ebce4dc7f16dfbL, 362},
        {0x8d3360f09cf6e4bdL, 366},  {0xb080392cc4349decL, 369},
        {0xdca04777f541c567L, 372},  {0x89e42caaf9491b60L, 376},
        {0xac5d37d5b79b6239L, 379},  {0xd77485cb25823ac7L, 382},
        {0x86a8d39ef77164bcL, 386},  {0xa8530886b54dbdebL, 389},
        {0xd267caa862a12d66L, 392},  {0x8380dea93da4bc60L, 396},
        {0xa46116538d0deb78L, 399},  {0xcd795be870516656L, 402},
        {0x806bd9714632dff6L, 406},  {0xa086cfcd97bf97f3L, 409},
        {0xc8a883c0fdaf7df0L, 412},  {0xfad2a4b13d1b5d6cL, 415},
        {0x9cc3a6eec6311a63L, 419},  {0xc3f490aa77bd60fcL, 422},
        {0xf4f1b4d515acb93bL, 425},  {0x991711052d8bf3c5L, 429},
        {0xbf5cd54678eef0b6L, 432},  {0xef340a98172aace4L, 435},
        {0x9580869f0e7aac0eL, 439},  {0xbae0a846d2195712L, 442},
        {0xe998d258869facd7L, 445},  {0x91ff83775423cc06L, 449},
        {0xb67f6455292cbf08L, 452},  {0xe41f3d6a7377eecaL, 455},
        {0x8e938662882af53eL, 459},  {0xb23867fb2a35b28dL, 462},
        {0xdec681f9f4c31f31L, 465},  {0x8b3c113c38f9f37eL, 469},
        {0xae0b158b4738705eL, 472},  {0xd98ddaee19068c76L, 475},
        {0x87f8a8d4cfa417c9L, 479},  {0xa9f6d30a038d1dbcL, 482},
        {0xd47487cc8470652bL, 485},  {0x84c8d4dfd2c63f3bL, 489},
        {0xa5fb0a17c777cf09L, 492},  {0xcf79cc9db955c2ccL, 495},
        {0x81ac1fe293d599bfL, 499},  {0xa21727db38cb002fL, 502},
        {0xca9cf1d206fdc03bL, 505},  {0xfd442e4688bd304aL, 508},
        {0x9e4a9cec15763e2eL, 512},  {0xc5dd44271ad3cdbaL, 515},
        {0xf7549530e188c128L, 518},  {0x9a94dd3e8cf578b9L, 522},
        {0xc13a148e3032d6e7L, 525},  {0xf18899b1bc3f8ca1L, 528},
        {0x96f5600f15a7b7e5L, 532},  {0xbcb2b812db11a5deL, 535},
        {0xebdf661791d60f56L, 538},  {0x936b9fcebb25c995L, 542},
        {0xb84687c269ef3bfbL, 545},  {0xe65829b3046b0afaL, 548},
        {0x8ff71a0fe2c2e6dcL, 552},  {0xb3f4e093db73a093L, 555},
        {0xe0f218b8d25088b8L, 558},  {0x8c974f7383725573L, 562},
        {0xafbd2350644eeacfL, 565},  {0xdbac6c247d62a583L, 568},
        {0x894bc396ce5da772L, 572},  {0xab9eb47c81f5114fL, 575},
        {0xd686619ba27255a2L, 578},  {0x8613fd0145877585L, 582},
        {0xa798fc4196e952e7L, 585},  {0xd17f3b51fca3a7a0L, 588},
        {0x82ef85133de648c4L, 592},  {0xa3ab66580d5fdaf5L, 595},
        {0xcc963fee10b7d1b3L, 598},  {0xffbbcfe994e5c61fL, 601},
        {0x9fd561f1fd0f9bd3L, 605},  {0xc7caba6e7c5382c8L, 608},
        {0xf9bd690a1b68637bL, 611},  {0x9c1661a651213e2dL, 615},
        {0xc31bfa0fe5698db8L, 618},  {0xf3e2f893dec3f126L, 621},
        {0x986ddb5c6b3a76b7L, 625},  {0xbe89523386091465L, 628},
        {0xee2ba6c0678b597fL, 631},  {0x94db483840b717efL, 635},
        {0xba121a4650e4ddebL, 638},  {0xe896a0d7e51e1566L, 641},
        {0x915e2486ef32cd60L, 645},  {0xb5b5ada8aaff80b8L, 648},
        {0xe3231912d5bf60e6L, 651},  {0x8df5efabc5979c8fL, 655},
        {0xb1736b96b6fd83b3L, 658},  {0xddd0467c64bce4a0L, 661},
        {0x8aa22c0dbef60ee4L, 665},  {0xad4ab7112eb3929dL, 668},
        {0xd89d64d57a607744L, 671},  {0x87625f056c7c4a8bL, 675},
        {0xa93af6c6c79b5d2dL, 678},  {0xd389b47879823479L, 681},
        {0x843610cb4bf160cbL, 685},  {0xa54394fe1eedb8feL, 688},
        {0xce947a3da6a9273eL, 691},  {0x811ccc668829b887L, 695},
        {0xa163ff802a3426a8L, 698},  {0xc9bcff6034c13052L, 701},
        {0xfc2c3f3841f17c67L, 704},  {0x9d9ba7832936edc0L, 708},
        {0xc5029163f384a931L, 711},  {0xf64335bcf065d37dL, 714},
        {0x99ea0196163fa42eL, 718},  {0xc06481fb9bcf8d39L, 721},
        {0xf07da27a82c37088L, 724},  {0x964e858c91ba2655L, 728},
        {0xbbe226efb628afeaL, 731},  {0xeadab0aba3b2dbe5L, 734},
        {0x92c8ae6b464fc96fL, 738},  {0xb77ada0617e3bbcbL, 741},
        {0xe55990879ddcaabdL, 744},  {0x8f57fa54c2a9eab6L, 748},
        {0xb32df8e9f3546564L, 751},  {0xdff9772470297ebdL, 754},
        {0x8bfbea76c619ef36L, 758},  {0xaefae51477a06b03L, 761},
        {0xdab99e59958885c4L, 764},  {0x88b402f7fd75539bL, 768},
        {0xaae103b5fcd2a881L, 771},  {0xd59944a37c0752a2L, 774},
        {0x857fcae62d8493a5L, 778},  {0xa6dfbd9fb8e5b88eL, 781},
        {0xd097ad07a71f26b2L, 784},  {0x825ecc24c873782fL, 788},
        {0xa2f67f2dfa90563bL, 791},  {0xcbb41ef979346bcaL, 794},
        {0xfea126b7d78186bcL, 797},  {0x9f24b832e6b0f436L, 801},
        {0xc6ede63fa05d3143L, 804},  {0xf8a95fcf88747d94L, 807},
        {0x9b69dbe1b548ce7cL, 811},  {0xc24452da229b021bL, 814},
        {0xf2d56790ab41c2a2L, 817},  {0x97c560ba6b0919a5L, 821},
        {0xbdb6b8e905cb600fL, 824},  {0xed246723473e3813L, 827},
        {0x9436c0760c86e30bL, 831},  {0xb94470938fa89bceL, 834},
        {0xe7958cb87392c2c2L, 837},  {0x90bd77f3483bb9b9L, 841},
        {0xb4ecd5f01a4aa828L, 844},  {0xe2280b6c20dd5232L, 847},
        {0x8d590723948a535fL, 851},  {0xb0af48ec79ace837L, 854},
        {0xdcdb1b2798182244L, 857},  {0x8a08f0f8bf0f156bL, 861},
        {0xac8b2d36eed2dac5L, 864},  {0xd7adf884aa879177L, 867},
        {0x86ccbb52ea94baeaL, 871},  {0xa87fea27a539e9a5L, 874},
        {0xd29fe4b18e88640eL, 877},  {0x83a3eeeef9153e89L, 881},
        {0xa48ceaaab75a8e2bL, 884},  {0xcdb02555653131b6L, 887},
        {0x808e17555f3ebf11L, 891},  {0xa0b19d2ab70e6ed6L, 894},
        {0xc8de047564d20a8bL, 897},  {0xfb158592be068d2eL, 900},
        {0x9ced737bb6c4183dL, 904},  {0xc428d05aa4751e4cL, 907},
        {0xf53304714d9265dfL, 910},  {0x993fe2c6d07b7fabL, 914},
        {0xbf8fdb78849a5f96L, 917},  {0xef73d256a5c0f77cL, 920},
        {0x95a8637627989aadL, 924},  {0xbb127c53b17ec159L, 927},
        {0xe9d71b689dde71afL, 930},  {0x9226712162ab070dL, 934},
        {0xb6b00d69bb55c8d1L, 937},  {0xe45c10c42a2b3b05L, 940},
        {0x8eb98a7a9a5b04e3L, 944},  {0xb267ed1940f1c61cL, 947},
        {0xdf01e85f912e37a3L, 950},  {0x8b61313bbabce2c6L, 954},
        {0xae397d8aa96c1b77L, 957},  {0xd9c7dced53c72255L, 960},
        {0x881cea14545c7575L, 964},  {0xaa242499697392d2L, 967},
        {0xd4ad2dbfc3d07787L, 970},  {0x84ec3c97da624ab4L, 974},
        {0xa6274bbdd0fadd61L, 977},  {0xcfb11ead453994baL, 980},
        {0x81ceb32c4b43fcf4L, 984},  {0xa2425ff75e14fc31L, 987},
        {0xcad2f7f5359a3b3eL, 990},  {0xfd87b5f28300ca0dL, 993},
        {0x9e74d1b791e07e48L, 997},  {0xc612062576589ddaL, 1000},
        {0xf79687aed3eec551L, 1003}, {0x9abe14cd44753b52L, 1007},
        {0xc16d9a0095928a27L, 1010}, {0xf1c90080baf72cb1L, 1013},
        {0x971da05074da7beeL, 1017}, {0xbce5086492111aeaL, 1020},
        {0xec1e4a7db69561a5L, 1023}, {0x9392ee8e921d5d07L, 1027},
        {0xb877aa3236a4b449L, 1030}, {0xe69594bec44de15bL, 1033},
        {0x901d7cf73ab0acd9L, 1037}, {0xb424dc35095cd80fL, 1040},
        {0xe12e13424bb40e13L, 1043}, {0x8cbccc096f5088cbL, 1047},
        {0xafebff0bcb24aafeL, 1050}, {0xdbe6fecebdedd5beL, 1053},
        {0x89705f4136b4a597L, 1057}, {0xabcc77118461cefcL, 1060},
        {0xd6bf94d5e57a42bcL, 1063}, {0x8637bd05af6c69b5L, 1067},
        {0xa7c5ac471b478423L, 1070}, {0xd1b71758e219652bL, 1073},
        {0x83126e978d4fdf3bL, 1077}, {0xa3d70a3d70a3d70aL, 1080},
        {0xccccccccccccccccL, 1083}, {0x8000000000000000L, 1087},
        {0xa000000000000000L, 1090}, {0xc800000000000000L, 1093},
        {0xfa00000000000000L, 1096}, {0x9c40000000000000L, 1100},
        {0xc350000000000000L, 1103}, {0xf424000000000000L, 1106},
        {0x9896800000000000L, 1110}, {0xbebc200000000000L, 1113},
        {0xee6b280000000000L, 1116}, {0x9502f90000000000L, 1120},
        {0xba43b74000000000L, 1123}, {0xe8d4a51000000000L, 1126},
        {0x9184e72a00000000L, 1130}, {0xb5e620f480000000L, 1133},
        {0xe35fa931a0000000L, 1136}, {0x8e1bc9bf04000000L, 1140},
        {0xb1a2bc2ec5000000L, 1143}, {0xde0b6b3a76400000L, 1146},
        {0x8ac7230489e80000L, 1150}, {0xad78ebc5ac620000L, 1153},
        {0xd8d726b7177a8000L, 1156}, {0x878678326eac9000L, 1160},
        {0xa968163f0a57b400L, 1163}, {0xd3c21bcecceda100L, 1166},
        {0x84595161401484a0L, 1170}, {0xa56fa5b99019a5c8L, 1173},
        {0xcecb8f27f4200f3aL, 1176}, {0x813f3978f8940984L, 1180},
        {0xa18f07d736b90be5L, 1183}, {0xc9f2c9cd04674edeL, 1186},
        {0xfc6f7c4045812296L, 1189}, {0x9dc5ada82b70b59dL, 1193},
        {0xc5371912364ce305L, 1196}, {0xf684df56c3e01bc6L, 1199},
        {0x9a130b963a6c115cL, 1203}, {0xc097ce7bc90715b3L, 1206},
        {0xf0bdc21abb48db20L, 1209}, {0x96769950b50d88f4L, 1213},
        {0xbc143fa4e250eb31L, 1216}, {0xeb194f8e1ae525fdL, 1219},
        {0x92efd1b8d0cf37beL, 1223}, {0xb7abc627050305adL, 1226},
        {0xe596b7b0c643c719L, 1229}, {0x8f7e32ce7bea5c6fL, 1233},
        {0xb35dbf821ae4f38bL, 1236}, {0xe0352f62a19e306eL, 1239},
        {0x8c213d9da502de45L, 1243}, {0xaf298d050e4395d6L, 1246},
        {0xdaf3f04651d47b4cL, 1249}, {0x88d8762bf324cd0fL, 1253},
        {0xab0e93b6efee0053L, 1256}, {0xd5d238a4abe98068L, 1259},
        {0x85a36366eb71f041L, 1263}, {0xa70c3c40a64e6c51L, 1266},
        {0xd0cf4b50cfe20765L, 1269}, {0x82818f1281ed449fL, 1273},
        {0xa321f2d7226895c7L, 1276}, {0xcbea6f8ceb02bb39L, 1279},
        {0xfee50b7025c36a08L, 1282}, {0x9f4f2726179a2245L, 1286},
        {0xc722f0ef9d80aad6L, 1289}, {0xf8ebad2b84e0d58bL, 1292},
        {0x9b934c3b330c8577L, 1296}, {0xc2781f49ffcfa6d5L, 1299},
        {0xf316271c7fc3908aL, 1302}, {0x97edd871cfda3a56L, 1306},
        {0xbde94e8e43d0c8ecL, 1309}, {0xed63a231d4c4fb27L, 1312},
        {0x945e455f24fb1cf8L, 1316}, {0xb975d6b6ee39e436L, 1319},
        {0xe7d34c64a9c85d44L, 1322}, {0x90e40fbeea1d3a4aL, 1326},
        {0xb51d13aea4a488ddL, 1329}, {0xe264589a4dcdab14L, 1332},
        {0x8d7eb76070a08aecL, 1336}, {0xb0de65388cc8ada8L, 1339},
        {0xdd15fe86affad912L, 1342}, {0x8a2dbf142dfcc7abL, 1346},
        {0xacb92ed9397bf996L, 1349}, {0xd7e77a8f87daf7fbL, 1352},
        {0x86f0ac99b4e8dafdL, 1356}, {0xa8acd7c0222311bcL, 1359},
        {0xd2d80db02aabd62bL, 1362}, {0x83c7088e1aab65dbL, 1366},
        {0xa4b8cab1a1563f52L, 1369}, {0xcde6fd5e09abcf26L, 1372},
        {0x80b05e5ac60b6178L, 1376}, {0xa0dc75f1778e39d6L, 1379},
        {0xc913936dd571c84cL, 1382}, {0xfb5878494ace3a5fL, 1385},
        {0x9d174b2dcec0e47bL, 1389}, {0xc45d1df942711d9aL, 1392},
        {0xf5746577930d6500L, 1395}, {0x9968bf6abbe85f20L, 1399},
        {0xbfc2ef456ae276e8L, 1402}, {0xefb3ab16c59b14a2L, 1405},
        {0x95d04aee3b80ece5L, 1409}, {0xbb445da9ca61281fL, 1412},
        {0xea1575143cf97226L, 1415}, {0x924d692ca61be758L, 1419},
        {0xb6e0c377cfa2e12eL, 1422}, {0xe498f455c38b997aL, 1425},
        {0x8edf98b59a373fecL, 1429}, {0xb2977ee300c50fe7L, 1432},
        {0xdf3d5e9bc0f653e1L, 1435}, {0x8b865b215899f46cL, 1439},
        {0xae67f1e9aec07187L, 1442}, {0xda01ee641a708de9L, 1445},
        {0x884134fe908658b2L, 1449}, {0xaa51823e34a7eedeL, 1452},
        {0xd4e5e2cdc1d1ea96L, 1455}, {0x850fadc09923329eL, 1459},
        {0xa6539930bf6bff45L, 1462}, {0xcfe87f7cef46ff16L, 1465},
        {0x81f14fae158c5f6eL, 1469}, {0xa26da3999aef7749L, 1472},
        {0xcb090c8001ab551cL, 1475}, {0xfdcb4fa002162a63L, 1478},
        {0x9e9f11c4014dda7eL, 1482}, {0xc646d63501a1511dL, 1485},
        {0xf7d88bc24209a565L, 1488}, {0x9ae757596946075fL, 1492},
        {0xc1a12d2fc3978937L, 1495}, {0xf209787bb47d6b84L, 1498},
        {0x9745eb4d50ce6332L, 1502}, {0xbd176620a501fbffL, 1505},
        {0xec5d3fa8ce427affL, 1508}, {0x93ba47c980e98cdfL, 1512},
        {0xb8a8d9bbe123f017L, 1515}, {0xe6d3102ad96cec1dL, 1518},
        {0x9043ea1ac7e41392L, 1522}, {0xb454e4a179dd1877L, 1525},
        {0xe16a1dc9d8545e94L, 1528}, {0x8ce2529e2734bb1dL, 1532},
        {0xb01ae745b101e9e4L, 1535}, {0xdc21a1171d42645dL, 1538},
        {0x899504ae72497ebaL, 1542}, {0xabfa45da0edbde69L, 1545},
        {0xd6f8d7509292d603L, 1548}, {0x865b86925b9bc5c2L, 1552},
        {0xa7f26836f282b732L, 1555}, {0xd1ef0244af2364ffL, 1558},
        {0x8335616aed761f1fL, 1562}, {0xa402b9c5a8d3a6e7L, 1565},
        {0xcd036837130890a1L, 1568}, {0x802221226be55a64L, 1572},
        {0xa02aa96b06deb0fdL, 1575}, {0xc83553c5c8965d3dL, 1578},
        {0xfa42a8b73abbf48cL, 1581}, {0x9c69a97284b578d7L, 1585},
        {0xc38413cf25e2d70dL, 1588}, {0xf46518c2ef5b8cd1L, 1591},
        {0x98bf2f79d5993802L, 1595}, {0xbeeefb584aff8603L, 1598},
        {0xeeaaba2e5dbf6784L, 1601}, {0x952ab45cfa97a0b2L, 1605},
        {0xba756174393d88dfL, 1608}, {0xe912b9d1478ceb17L, 1611},
        {0x91abb422ccb812eeL, 1615}, {0xb616a12b7fe617aaL, 1618},
        {0xe39c49765fdf9d94L, 1621}, {0x8e41ade9fbebc27dL, 1625},
        {0xb1d219647ae6b31cL, 1628}, {0xde469fbd99a05fe3L, 1631},
        {0x8aec23d680043beeL, 1635}, {0xada72ccc20054ae9L, 1638},
        {0xd910f7ff28069da4L, 1641}, {0x87aa9aff79042286L, 1645},
        {0xa99541bf57452b28L, 1648}, {0xd3fa922f2d1675f2L, 1651},
        {0x847c9b5d7c2e09b7L, 1655}, {0xa59bc234db398c25L, 1658},
        {0xcf02b2c21207ef2eL, 1661}, {0x8161afb94b44f57dL, 1665},
        {0xa1ba1ba79e1632dcL, 1668}, {0xca28a291859bbf93L, 1671},
        {0xfcb2cb35e702af78L, 1674}, {0x9defbf01b061adabL, 1678},
        {0xc56baec21c7a1916L, 1681}, {0xf6c69a72a3989f5bL, 1684},
        {0x9a3c2087a63f6399L, 1688}, {0xc0cb28a98fcf3c7fL, 1691},
        {0xf0fdf2d3f3c30b9fL, 1694}, {0x969eb7c47859e743L, 1698},
        {0xbc4665b596706114L, 1701}, {0xeb57ff22fc0c7959L, 1704},
        {0x9316ff75dd87cbd8L, 1708}, {0xb7dcbf5354e9beceL, 1711},
        {0xe5d3ef282a242e81L, 1714}, {0x8fa475791a569d10L, 1718},
        {0xb38d92d760ec4455L, 1721}, {0xe070f78d3927556aL, 1724},
        {0x8c469ab843b89562L, 1728}, {0xaf58416654a6babbL, 1731},
        {0xdb2e51bfe9d0696aL, 1734}, {0x88fcf317f22241e2L, 1738},
        {0xab3c2fddeeaad25aL, 1741}, {0xd60b3bd56a5586f1L, 1744},
        {0x85c7056562757456L, 1748}, {0xa738c6bebb12d16cL, 1751},
        {0xd106f86e69d785c7L, 1754}, {0x82a45b450226b39cL, 1758},
        {0xa34d721642b06084L, 1761}, {0xcc20ce9bd35c78a5L, 1764},
        {0xff290242c83396ceL, 1767}, {0x9f79a169bd203e41L, 1771},
        {0xc75809c42c684dd1L, 1774}, {0xf92e0c3537826145L, 1777},
        {0x9bbcc7a142b17ccbL, 1781}, {0xc2abf989935ddbfeL, 1784},
        {0xf356f7ebf83552feL, 1787}, {0x98165af37b2153deL, 1791},
        {0xbe1bf1b059e9a8d6L, 1794}, {0xeda2ee1c7064130cL, 1797},
        {0x9485d4d1c63e8be7L, 1801}, {0xb9a74a0637ce2ee1L, 1804},
        {0xe8111c87c5c1ba99L, 1807}, {0x910ab1d4db9914a0L, 1811},
        {0xb54d5e4a127f59c8L, 1814}, {0xe2a0b5dc971f303aL, 1817},
        {0x8da471a9de737e24L, 1821}, {0xb10d8e1456105dadL, 1824},
        {0xdd50f1996b947518L, 1827}, {0x8a5296ffe33cc92fL, 1831},
        {0xace73cbfdc0bfb7bL, 1834}, {0xd8210befd30efa5aL, 1837},
        {0x8714a775e3e95c78L, 1841}, {0xa8d9d1535ce3b396L, 1844},
        {0xd31045a8341ca07cL, 1847}, {0x83ea2b892091e44dL, 1851},
        {0xa4e4b66b68b65d60L, 1854}, {0xce1de40642e3f4b9L, 1857},
        {0x80d2ae83e9ce78f3L, 1861}, {0xa1075a24e4421730L, 1864},
        {0xc94930ae1d529cfcL, 1867}, {0xfb9b7cd9a4a7443cL, 1870},
        {0x9d412e0806e88aa5L, 1874}, {0xc491798a08a2ad4eL, 1877},
        {0xf5b5d7ec8acb58a2L, 1880}, {0x9991a6f3d6bf1765L, 1884},
        {0xbff610b0cc6edd3fL, 1887}, {0xeff394dcff8a948eL, 1890},
        {0x95f83d0a1fb69cd9L, 1894}, {0xbb764c4ca7a4440fL, 1897},
        {0xea53df5fd18d5513L, 1900}, {0x92746b9be2f8552cL, 1904},
        {0xb7118682dbb66a77L, 1907}, {0xe4d5e82392a40515L, 1910},
        {0x8f05b1163ba6832dL, 1914}, {0xb2c71d5bca9023f8L, 1917},
        {0xdf78e4b2bd342cf6L, 1920}, {0x8bab8eefb6409c1aL, 1924},
        {0xae9672aba3d0c320L, 1927}, {0xda3c0f568cc4f3e8L, 1930},
        {0x8865899617fb1871L, 1934}, {0xaa7eebfb9df9de8dL, 1937},
        {0xd51ea6fa85785631L, 1940}, {0x8533285c936b35deL, 1944},
        {0xa67ff273b8460356L, 1947}, {0xd01fef10a657842cL, 1950},
        {0x8213f56a67f6b29bL, 1954}, {0xa298f2c501f45f42L, 1957},
        {0xcb3f2f7642717713L, 1960}, {0xfe0efb53d30dd4d7L, 1963},
        {0x9ec95d1463e8a506L, 1967}, {0xc67bb4597ce2ce48L, 1970},
        {0xf81aa16fdc1b81daL, 1973}, {0x9b10a4e5e9913128L, 1977},
        {0xc1d4ce1f63f57d72L, 1980}, {0xf24a01a73cf2dccfL, 1983},
        {0x976e41088617ca01L, 1987}, {0xbd49d14aa79dbc82L, 1990},
        {0xec9c459d51852ba2L, 1993}, {0x93e1ab8252f33b45L, 1997},
        {0xb8da1662e7b00a17L, 2000}, {0xe7109bfba19c0c9dL, 2003},
        {0x906a617d450187e2L, 2007}, {0xb484f9dc9641e9daL, 2010},
        {0xe1a63853bbd26451L, 2013}, {0x8d07e33455637eb2L, 2017},
        {0xb049dc016abc5e5fL, 2020}, {0xdc5c5301c56b75f7L, 2023},
        {0x89b9b3e11b6329baL, 2027}, {0xac2820d9623bf429L, 2030},
        {0xd732290fbacaf133L, 2033}, {0x867f59a9d4bed6c0L, 2037},
        {0xa81f301449ee8c70L, 2040}, {0xd226fc195c6a2f8cL, 2043},
        {0x83585d8fd9c25db7L, 2047}, {0xa42e74f3d032f525L, 2050},
        {0xcd3a1230c43fb26fL, 2053}, {0x80444b5e7aa7cf85L, 2057},
        {0xa0555e361951c366L, 2060}, {0xc86ab5c39fa63440L, 2063},
        {0xfa856334878fc150L, 2066}, {0x9c935e00d4b9d8d2L, 2070},
        {0xc3b8358109e84f07L, 2073}, {0xf4a642e14c6262c8L, 2076},
        {0x98e7e9cccfbd7dbdL, 2080}, {0xbf21e44003acdd2cL, 2083},
        {0xeeea5d5004981478L, 2086}, {0x95527a5202df0ccbL, 2090},
        {0xbaa718e68396cffdL, 2093}, {0xe950df20247c83fdL, 2096},
        {0x91d28b7416cdd27eL, 2100}, {0xb6472e511c81471dL, 2103},
        {0xe3d8f9e563a198e5L, 2106}, {0x8e679c2f5e44ff8fL, 2110} };

    // A complement from power_of_ten_components
    // complete to a 128-bit mantissa.
    const uint64_t mantissa_128[] = {
        0x419ea3bd35385e2d,
        0x52064cac828675b9,
        0x7343efebd1940993,
        0x1014ebe6c5f90bf8,
        0xd41a26e077774ef6,
        0x8920b098955522b4,
        0x55b46e5f5d5535b0,
        0xeb2189f734aa831d,
        0xa5e9ec7501d523e4,
        0x47b233c92125366e,
        0x999ec0bb696e840a,
        0xc00670ea43ca250d,
        0x380406926a5e5728,
        0xc605083704f5ecf2,
        0xf7864a44c633682e,
        0x7ab3ee6afbe0211d,
        0x5960ea05bad82964,
        0x6fb92487298e33bd,
        0xa5d3b6d479f8e056,
        0x8f48a4899877186c,
        0x331acdabfe94de87,
        0x9ff0c08b7f1d0b14,
        0x7ecf0ae5ee44dd9,
        0xc9e82cd9f69d6150,
        0xbe311c083a225cd2,
        0x6dbd630a48aaf406,
        0x92cbbccdad5b108,
        0x25bbf56008c58ea5,
        0xaf2af2b80af6f24e,
        0x1af5af660db4aee1,
        0x50d98d9fc890ed4d,
        0xe50ff107bab528a0,
        0x1e53ed49a96272c8,
        0x25e8e89c13bb0f7a,
        0x77b191618c54e9ac,
        0xd59df5b9ef6a2417,
        0x4b0573286b44ad1d,
        0x4ee367f9430aec32,
        0x229c41f793cda73f,
        0x6b43527578c1110f,
        0x830a13896b78aaa9,
        0x23cc986bc656d553,
        0x2cbfbe86b7ec8aa8,
        0x7bf7d71432f3d6a9,
        0xdaf5ccd93fb0cc53,
        0xd1b3400f8f9cff68,
        0x23100809b9c21fa1,
        0xabd40a0c2832a78a,
        0x16c90c8f323f516c,
        0xae3da7d97f6792e3,
        0x99cd11cfdf41779c,
        0x40405643d711d583,
        0x482835ea666b2572,
        0xda3243650005eecf,
        0x90bed43e40076a82,
        0x5a7744a6e804a291,
        0x711515d0a205cb36,
        0xd5a5b44ca873e03,
        0xe858790afe9486c2,
        0x626e974dbe39a872,
        0xfb0a3d212dc8128f,
        0x7ce66634bc9d0b99,
        0x1c1fffc1ebc44e80,
        0xa327ffb266b56220,
        0x4bf1ff9f0062baa8,
        0x6f773fc3603db4a9,
        0xcb550fb4384d21d3,
        0x7e2a53a146606a48,
        0x2eda7444cbfc426d,
        0xfa911155fefb5308,
        0x793555ab7eba27ca,
        0x4bc1558b2f3458de,
        0x9eb1aaedfb016f16,
        0x465e15a979c1cadc,
        0xbfacd89ec191ec9,
        0xcef980ec671f667b,
        0x82b7e12780e7401a,
        0xd1b2ecb8b0908810,
        0x861fa7e6dcb4aa15,
        0x67a791e093e1d49a,
        0xe0c8bb2c5c6d24e0,
        0x58fae9f773886e18,
        0xaf39a475506a899e,
        0x6d8406c952429603,
        0xc8e5087ba6d33b83,
        0xfb1e4a9a90880a64,
        0x5cf2eea09a55067f,
        0xf42faa48c0ea481e,
        0xf13b94daf124da26,
        0x76c53d08d6b70858,
        0x54768c4b0c64ca6e,
        0xa9942f5dcf7dfd09,
        0xd3f93b35435d7c4c,
        0xc47bc5014a1a6daf,
        0x359ab6419ca1091b,
        0xc30163d203c94b62,
        0x79e0de63425dcf1d,
        0x985915fc12f542e4,
        0x3e6f5b7b17b2939d,
        0xa705992ceecf9c42,
        0x50c6ff782a838353,
        0xa4f8bf5635246428,
        0x871b7795e136be99,
        0x28e2557b59846e3f,
        0x331aeada2fe589cf,
        0x3ff0d2c85def7621,
        0xfed077a756b53a9,
        0xd3e8495912c62894,
        0x64712dd7abbbd95c,
        0xbd8d794d96aacfb3,
        0xecf0d7a0fc5583a0,
        0xf41686c49db57244,
        0x311c2875c522ced5,
        0x7d633293366b828b,
        0xae5dff9c02033197,
        0xd9f57f830283fdfc,
        0xd072df63c324fd7b,
        0x4247cb9e59f71e6d,
        0x52d9be85f074e608,
        0x67902e276c921f8b,
        0xba1cd8a3db53b6,
        0x80e8a40eccd228a4,
        0x6122cd128006b2cd,
        0x796b805720085f81,
        0xcbe3303674053bb0,
        0xbedbfc4411068a9c,
        0xee92fb5515482d44,
        0x751bdd152d4d1c4a,
        0xd262d45a78a0635d,
        0x86fb897116c87c34,
        0xd45d35e6ae3d4da0,
        0x8974836059cca109,
        0x2bd1a438703fc94b,
        0x7b6306a34627ddcf,
        0x1a3bc84c17b1d542,
        0x20caba5f1d9e4a93,
        0x547eb47b7282ee9c,
        0xe99e619a4f23aa43,
        0x6405fa00e2ec94d4,
        0xde83bc408dd3dd04,
        0x9624ab50b148d445,
        0x3badd624dd9b0957,
        0xe54ca5d70a80e5d6,
        0x5e9fcf4ccd211f4c,
        0x7647c3200069671f,
        0x29ecd9f40041e073,
        0xf468107100525890,
        0x7182148d4066eeb4,
        0xc6f14cd848405530,
        0xb8ada00e5a506a7c,
        0xa6d90811f0e4851c,
        0x908f4a166d1da663,
        0x9a598e4e043287fe,
        0x40eff1e1853f29fd,
        0xd12bee59e68ef47c,
        0x82bb74f8301958ce,
        0xe36a52363c1faf01,
        0xdc44e6c3cb279ac1,
        0x29ab103a5ef8c0b9,
        0x7415d448f6b6f0e7,
        0x111b495b3464ad21,
        0xcab10dd900beec34,
        0x3d5d514f40eea742,
        0xcb4a5a3112a5112,
        0x47f0e785eaba72ab,
        0x59ed216765690f56,
        0x306869c13ec3532c,
        0x1e414218c73a13fb,
        0xe5d1929ef90898fa,
        0xdf45f746b74abf39,
        0x6b8bba8c328eb783,
        0x66ea92f3f326564,
        0xc80a537b0efefebd,
        0xbd06742ce95f5f36,
        0x2c48113823b73704,
        0xf75a15862ca504c5,
        0x9a984d73dbe722fb,
        0xc13e60d0d2e0ebba,
        0x318df905079926a8,
        0xfdf17746497f7052,
        0xfeb6ea8bedefa633,
        0xfe64a52ee96b8fc0,
        0x3dfdce7aa3c673b0,
        0x6bea10ca65c084e,
        0x486e494fcff30a62,
        0x5a89dba3c3efccfa,
        0xf89629465a75e01c,
        0xf6bbb397f1135823,
        0x746aa07ded582e2c,
        0xa8c2a44eb4571cdc,
        0x92f34d62616ce413,
        0x77b020baf9c81d17,
        0xace1474dc1d122e,
        0xd819992132456ba,
        0x10e1fff697ed6c69,
        0xca8d3ffa1ef463c1,
        0xbd308ff8a6b17cb2,
        0xac7cb3f6d05ddbde,
        0x6bcdf07a423aa96b,
        0x86c16c98d2c953c6,
        0xe871c7bf077ba8b7,
        0x11471cd764ad4972,
        0xd598e40d3dd89bcf,
        0x4aff1d108d4ec2c3,
        0xcedf722a585139ba,
        0xc2974eb4ee658828,
        0x733d226229feea32,
        0x806357d5a3f525f,
        0xca07c2dcb0cf26f7,
        0xfc89b393dd02f0b5,
        0xbbac2078d443ace2,
        0xd54b944b84aa4c0d,
        0xa9e795e65d4df11,
        0x4d4617b5ff4a16d5,
        0x504bced1bf8e4e45,
        0xe45ec2862f71e1d6,
        0x5d767327bb4e5a4c,
        0x3a6a07f8d510f86f,
        0x890489f70a55368b,
        0x2b45ac74ccea842e,
        0x3b0b8bc90012929d,
        0x9ce6ebb40173744,
        0xcc420a6a101d0515,
        0x9fa946824a12232d,
        0x47939822dc96abf9,
        0x59787e2b93bc56f7,
        0x57eb4edb3c55b65a,
        0xede622920b6b23f1,
        0xe95fab368e45eced,
        0x11dbcb0218ebb414,
        0xd652bdc29f26a119,
        0x4be76d3346f0495f,
        0x6f70a4400c562ddb,
        0xcb4ccd500f6bb952,
        0x7e2000a41346a7a7,
        0x8ed400668c0c28c8,
        0x728900802f0f32fa,
        0x4f2b40a03ad2ffb9,
        0xe2f610c84987bfa8,
        0xdd9ca7d2df4d7c9,
        0x91503d1c79720dbb,
        0x75a44c6397ce912a,
        0xc986afbe3ee11aba,
        0xfbe85badce996168,
        0xfae27299423fb9c3,
        0xdccd879fc967d41a,
        0x5400e987bbc1c920,
        0x290123e9aab23b68,
        0xf9a0b6720aaf6521,
        0xf808e40e8d5b3e69,
        0xb60b1d1230b20e04,
        0xb1c6f22b5e6f48c2,
        0x1e38aeb6360b1af3,
        0x25c6da63c38de1b0,
        0x579c487e5a38ad0e,
        0x2d835a9df0c6d851,
        0xf8e431456cf88e65,
        0x1b8e9ecb641b58ff,
        0xe272467e3d222f3f,
        0x5b0ed81dcc6abb0f,
        0x98e947129fc2b4e9,
        0x3f2398d747b36224,
        0x8eec7f0d19a03aad,
        0x1953cf68300424ac,
        0x5fa8c3423c052dd7,
        0x3792f412cb06794d,
        0xe2bbd88bbee40bd0,
        0x5b6aceaeae9d0ec4,
        0xf245825a5a445275,
        0xeed6e2f0f0d56712,
        0x55464dd69685606b,
        0xaa97e14c3c26b886,
        0xd53dd99f4b3066a8,
        0xe546a8038efe4029,
        0xde98520472bdd033,
        0x963e66858f6d4440,
        0xdde7001379a44aa8,
        0x5560c018580d5d52,
        0xaab8f01e6e10b4a6,
        0xcab3961304ca70e8,
        0x3d607b97c5fd0d22,
        0x8cb89a7db77c506a,
        0x77f3608e92adb242,
        0x55f038b237591ed3,
        0x6b6c46dec52f6688,
        0x2323ac4b3b3da015,
        0xabec975e0a0d081a,
        0x96e7bd358c904a21,
        0x7e50d64177da2e54,
        0xdde50bd1d5d0b9e9,
        0x955e4ec64b44e864,
        0xbd5af13bef0b113e,
        0xecb1ad8aeacdd58e,
        0x67de18eda5814af2,
        0x80eacf948770ced7,
        0xa1258379a94d028d,
        0x96ee45813a04330,
        0x8bca9d6e188853fc,
        0x775ea264cf55347d,
        0x95364afe032a819d,
        0x3a83ddbd83f52204,
        0xc4926a9672793542,
        0x75b7053c0f178293,
        0x5324c68b12dd6338,
        0xd3f6fc16ebca5e03,
        0x88f4bb1ca6bcf584,
        0x2b31e9e3d06c32e5,
        0x3aff322e62439fcf,
        0x9befeb9fad487c2,
        0x4c2ebe687989a9b3,
        0xf9d37014bf60a10,
        0x538484c19ef38c94,
        0x2865a5f206b06fb9,
        0xf93f87b7442e45d3,
        0xf78f69a51539d748,
        0xb573440e5a884d1b,
        0x31680a88f8953030,
        0xfdc20d2b36ba7c3d,
        0x3d32907604691b4c,
        0xa63f9a49c2c1b10f,
        0xfcf80dc33721d53,
        0xd3c36113404ea4a8,
        0x645a1cac083126e9,
        0x3d70a3d70a3d70a3,
        0xcccccccccccccccc,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x4000000000000000,
        0x5000000000000000,
        0xa400000000000000,
        0x4d00000000000000,
        0xf020000000000000,
        0x6c28000000000000,
        0xc732000000000000,
        0x3c7f400000000000,
        0x4b9f100000000000,
        0x1e86d40000000000,
        0x1314448000000000,
        0x17d955a000000000,
        0x5dcfab0800000000,
        0x5aa1cae500000000,
        0xf14a3d9e40000000,
        0x6d9ccd05d0000000,
        0xe4820023a2000000,
        0xdda2802c8a800000,
        0xd50b2037ad200000,
        0x4526f422cc340000,
        0x9670b12b7f410000,
        0x3c0cdd765f114000,
        0xa5880a69fb6ac800,
        0x8eea0d047a457a00,
        0x72a4904598d6d880,
        0x47a6da2b7f864750,
        0x999090b65f67d924,
        0xfff4b4e3f741cf6d,
        0xbff8f10e7a8921a4,
        0xaff72d52192b6a0d,
        0x9bf4f8a69f764490,
        0x2f236d04753d5b4,
        0x1d762422c946590,
        0x424d3ad2b7b97ef5,
        0xd2e0898765a7deb2,
        0x63cc55f49f88eb2f,
        0x3cbf6b71c76b25fb,
        0x8bef464e3945ef7a,
        0x97758bf0e3cbb5ac,
        0x3d52eeed1cbea317,
        0x4ca7aaa863ee4bdd,
        0x8fe8caa93e74ef6a,
        0xb3e2fd538e122b44,
        0x60dbbca87196b616,
        0xbc8955e946fe31cd,
        0x6babab6398bdbe41,
        0xc696963c7eed2dd1,
        0xfc1e1de5cf543ca2,
        0x3b25a55f43294bcb,
        0x49ef0eb713f39ebe,
        0x6e3569326c784337,
        0x49c2c37f07965404,
        0xdc33745ec97be906,
        0x69a028bb3ded71a3,
        0xc40832ea0d68ce0c,
        0xf50a3fa490c30190,
        0x792667c6da79e0fa,
        0x577001b891185938,
        0xed4c0226b55e6f86,
        0x544f8158315b05b4,
        0x696361ae3db1c721,
        0x3bc3a19cd1e38e9,
        0x4ab48a04065c723,
        0x62eb0d64283f9c76,
        0x3ba5d0bd324f8394,
        0xca8f44ec7ee36479,
        0x7e998b13cf4e1ecb,
        0x9e3fedd8c321a67e,
        0xc5cfe94ef3ea101e,
        0xbba1f1d158724a12,
        0x2a8a6e45ae8edc97,
        0xf52d09d71a3293bd,
        0x593c2626705f9c56,
        0x6f8b2fb00c77836c,
        0xb6dfb9c0f956447,
        0x4724bd4189bd5eac,
        0x58edec91ec2cb657,
        0x2f2967b66737e3ed,
        0xbd79e0d20082ee74,
        0xecd8590680a3aa11,
        0xe80e6f4820cc9495,
        0x3109058d147fdcdd,
        0xbd4b46f0599fd415,
        0x6c9e18ac7007c91a,
        0x3e2cf6bc604ddb0,
        0x84db8346b786151c,
        0xe612641865679a63,
        0x4fcb7e8f3f60c07e,
        0xe3be5e330f38f09d,
        0x5cadf5bfd3072cc5,
        0x73d9732fc7c8f7f6,
        0x2867e7fddcdd9afa,
        0xb281e1fd541501b8,
        0x1f225a7ca91a4226,
        0x3375788de9b06958,
        0x52d6b1641c83ae,
        0xc0678c5dbd23a49a,
        0xf840b7ba963646e0,
        0xb650e5a93bc3d898,
        0xa3e51f138ab4cebe,
        0xc66f336c36b10137,
        0xb80b0047445d4184,
        0xa60dc059157491e5,
        0x87c89837ad68db2f,
        0x29babe4598c311fb,
        0xf4296dd6fef3d67a,
        0x1899e4a65f58660c,
        0x5ec05dcff72e7f8f,
        0x76707543f4fa1f73,
        0x6a06494a791c53a8,
        0x487db9d17636892,
        0x45a9d2845d3c42b6,
        0xb8a2392ba45a9b2,
        0x8e6cac7768d7141e,
        0x3207d795430cd926,
        0x7f44e6bd49e807b8,
        0x5f16206c9c6209a6,
        0x36dba887c37a8c0f,
        0xc2494954da2c9789,
        0xf2db9baa10b7bd6c,
        0x6f92829494e5acc7,
        0xcb772339ba1f17f9,
        0xff2a760414536efb,
        0xfef5138519684aba,
        0x7eb258665fc25d69,
        0xef2f773ffbd97a61,
        0xaafb550ffacfd8fa,
        0x95ba2a53f983cf38,
        0xdd945a747bf26183,
        0x94f971119aeef9e4,
        0x7a37cd5601aab85d,
        0xac62e055c10ab33a,
        0x577b986b314d6009,
        0xed5a7e85fda0b80b,
        0x14588f13be847307,
        0x596eb2d8ae258fc8,
        0x6fca5f8ed9aef3bb,
        0x25de7bb9480d5854,
        0xaf561aa79a10ae6a,
        0x1b2ba1518094da04,
        0x90fb44d2f05d0842,
        0x353a1607ac744a53,
        0x42889b8997915ce8,
        0x69956135febada11,
        0x43fab9837e699095,
        0x94f967e45e03f4bb,
        0x1d1be0eebac278f5,
        0x6462d92a69731732,
        0x7d7b8f7503cfdcfe,
        0x5cda735244c3d43e,
        0x3a0888136afa64a7,
        0x88aaa1845b8fdd0,
        0x8aad549e57273d45,
        0x36ac54e2f678864b,
        0x84576a1bb416a7dd,
        0x656d44a2a11c51d5,
        0x9f644ae5a4b1b325,
        0x873d5d9f0dde1fee,
        0xa90cb506d155a7ea,
        0x9a7f12442d588f2,
        0xc11ed6d538aeb2f,
        0x8f1668c8a86da5fa,
        0xf96e017d694487bc,
        0x37c981dcc395a9ac,
        0x85bbe253f47b1417,
        0x93956d7478ccec8e,
        0x387ac8d1970027b2,
        0x6997b05fcc0319e,
        0x441fece3bdf81f03,
        0xd527e81cad7626c3,
        0x8a71e223d8d3b074,
        0xf6872d5667844e49,
        0xb428f8ac016561db,
        0xe13336d701beba52,
        0xecc0024661173473,
        0x27f002d7f95d0190,
        0x31ec038df7b441f4,
        0x7e67047175a15271,
        0xf0062c6e984d386,
        0x52c07b78a3e60868,
        0xa7709a56ccdf8a82,
        0x88a66076400bb691,
        0x6acff893d00ea435,
        0x583f6b8c4124d43,
        0xc3727a337a8b704a,
        0x744f18c0592e4c5c,
        0x1162def06f79df73,
        0x8addcb5645ac2ba8,
        0x6d953e2bd7173692,
        0xc8fa8db6ccdd0437,
        0x1d9c9892400a22a2,
        0x2503beb6d00cab4b,
        0x2e44ae64840fd61d,
        0x5ceaecfed289e5d2,
        0x7425a83e872c5f47,
        0xd12f124e28f77719,
        0x82bd6b70d99aaa6f,
        0x636cc64d1001550b,
        0x3c47f7e05401aa4e,
        0x65acfaec34810a71,
        0x7f1839a741a14d0d,
        0x1ede48111209a050,
        0x934aed0aab460432,
        0xf81da84d5617853f,
        0x36251260ab9d668e,
        0xc1d72b7c6b426019,
        0xb24cf65b8612f81f,
        0xdee033f26797b627,
        0x169840ef017da3b1,
        0x8e1f289560ee864e,
        0xf1a6f2bab92a27e2,
        0xae10af696774b1db,
        0xacca6da1e0a8ef29,
        0x17fd090a58d32af3,
        0xddfc4b4cef07f5b0,
        0x4abdaf101564f98e,
        0x9d6d1ad41abe37f1,
        0x84c86189216dc5ed,
        0x32fd3cf5b4e49bb4,
        0x3fbc8c33221dc2a1,
        0xfabaf3feaa5334a,
        0x29cb4d87f2a7400e,
        0x743e20e9ef511012,
        0x914da9246b255416,
        0x1ad089b6c2f7548e,
        0xa184ac2473b529b1,
        0xc9e5d72d90a2741e,
        0x7e2fa67c7a658892,
        0xddbb901b98feeab7,
        0x552a74227f3ea565,
        0xd53a88958f87275f,
        0x8a892abaf368f137,
        0x2d2b7569b0432d85,
        0x9c3b29620e29fc73,
        0x8349f3ba91b47b8f,
        0x241c70a936219a73,
        0xed238cd383aa0110,
        0xf4363804324a40aa,
        0xb143c6053edcd0d5,
        0xdd94b7868e94050a,
        0xca7cf2b4191c8326,
        0xfd1c2f611f63a3f0,
        0xbc633b39673c8cec,
        0xd5be0503e085d813,
        0x4b2d8644d8a74e18,
        0xddf8e7d60ed1219e,
        0xcabb90e5c942b503,
        0x3d6a751f3b936243,
        0xcc512670a783ad4,
        0x27fb2b80668b24c5,
        0xb1f9f660802dedf6,
        0x5e7873f8a0396973,
        0xdb0b487b6423e1e8,
        0x91ce1a9a3d2cda62,
        0x7641a140cc7810fb,
        0xa9e904c87fcb0a9d,
        0x546345fa9fbdcd44,
        0xa97c177947ad4095,
        0x49ed8eabcccc485d,
        0x5c68f256bfff5a74,
        0x73832eec6fff3111,
        0xc831fd53c5ff7eab,
        0xba3e7ca8b77f5e55,
        0x28ce1bd2e55f35eb,
        0x7980d163cf5b81b3,
        0xd7e105bcc332621f,
        0x8dd9472bf3fefaa7,
        0xb14f98f6f0feb951,
        0x6ed1bf9a569f33d3,
        0xa862f80ec4700c8,
        0xcd27bb612758c0fa,
        0x8038d51cb897789c,
        0xe0470a63e6bd56c3,
        0x1858ccfce06cac74,
        0xf37801e0c43ebc8,
        0xd30560258f54e6ba,
        0x47c6b82ef32a2069,
        0x4cdc331d57fa5441,
        0xe0133fe4adf8e952,
        0x58180fddd97723a6,
        0x570f09eaa7ea7648
    };

    // Attempts to compute i * 10^(power) exactly; and if "negative" is
    // true, negate the result.
    // This function will only work in some cases, when it does not work, success is
    // set to false. This should work *most of the time* (like 99% of the time).
    // We assume that power is in the [FASTFLOAT_SMALLEST_POWER,
    // FASTFLOAT_LARGEST_POWER] interval: the caller is responsible for this check.
    really_inline
    double compute_float_64(int64_t power, uint64_t i, bool negative, bool* success)
    {

        /*
            double dval = simd_double_parser::x_fast_path((double)i, power);
            if (negative) dval = -dval;
            *success = true;
            return dval;
        */

        // possible, except if i == 0, so we handle i == 0 separately.
        if (i == 0) {
            return 0.0;
        }

        // we start with a fast path
        // It was described in
        // Clinger WD. How to read floating point numbers accurately.
        // ACM SIGPLAN Notices. 1990
        if (-22 <= power && power <= 22 && i <= 9007199254740991) {
            // convert the integer into a double. This is lossless since
            // 0 <= i <= 2^53 - 1.
            double d = double(i);
            //
            // The general idea is as follows.
            // If 0 <= s < 2^53 and if 10^0 <= p <= 10^22 then
            // 1) Both s and p can be represented exactly as 64-bit floating-point
            // values
            // (binary64).
            // 2) Because s and p can be represented exactly as floating-point values,
            // then s * p
            // and s / p will produce correctly rounded values.
            //
            if (power < 0) {
                d = d / power_of_ten[-power];
            }
            else {
                d = d * power_of_ten[power];
            }
            if (negative) {
                d = -d;
            }
            *success = true;
            return d;
        }
        // When 22 < power && power <  22 + 16, we could
        // hope for another, secondary fast path.  It wa
        // described by David M. Gay in  "Correctly rounded
        // binary-decimal and decimal-binary conversions." (1990)
        // If you need to compute i * 10^(22 + x) for x < 16,
        // first compute i * 10^x, if you know that result is exact
        // (e.g., when i * 10^x < 2^53),
        // then you can still proceed and do (i * 10^x) * 10^22.
        // Is this worth your time?
        // You need  22 < power *and* power <  22 + 16 *and* (i * 10^(x-22) < 2^53)
        // for this second fast path to work.
        // If you you have 22 < power *and* power <  22 + 16, and then you
        // optimistically compute "i * 10^(x-22)", there is still a chance that you
        // have wasted your time if i * 10^(x-22) >= 2^53. It makes the use cases of
        // this optimization maybe less common than we would like. Source:
        // http://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/
        // also used in RapidJSON: https://rapidjson.org/strtod_8h_source.html



        // The fast path has now failed, so we are failing back on the slower path.

        // In the slow path, we need to adjust i so that it is > 1<<63 which is always

        // We are going to need to do some 64-bit arithmetic to get a more precise product.
        // We use a table lookup approach.
        components c =
            power_of_ten_components[power - FASTFLOAT_SMALLEST_POWER]; // safe because
                                                                       // power >= FASTFLOAT_SMALLEST_POWER
                                                                       // and power <= FASTFLOAT_LARGEST_POWER
        // we recover the mantissa of the power, it has a leading 1. It is always
        // rounded down.
        uint64_t factor_mantissa = c.mantissa;
        // We want the most significant bit of i to be 1. Shift if needed.
        int lz = leading_zeroes(i);
        i <<= lz;
        // We want the most significant 64 bits of the product. We know
        // this will be non-zero because the most significant bit of i is
        // 1.
        value128 product = full_multiplication(i, factor_mantissa);
        uint64_t lower = product.low;
        uint64_t upper = product.high;
        // We know that upper has at most one leading zero because
        // both i and  factor_mantissa have a leading one. This means
        // that the result is at least as large as ((1<<63)*(1<<63))/(1<<64).

        // As long as the first 9 bits of "upper" are not "1", then we
        // know that we have an exact computed value for the leading
        // 55 bits because any imprecision would play out as a +1, in
        // the worst case.
        // We expect this next branch to be rarely taken (say 1% of the time).
        // When (upper & 0x1FF) == 0x1FF, it can be common for
        // lower + i < lower to be true (proba. much higher than 1%).
        if (unlikely((upper & 0x1FF) == 0x1FF) && (lower + i < lower)) {
            uint64_t factor_mantissa_low =
                mantissa_128[power - FASTFLOAT_SMALLEST_POWER];
            // next, we compute the 64-bit x 128-bit multiplication, getting a 192-bit
            // result (three 64-bit values)
            product = full_multiplication(i, factor_mantissa_low);
            uint64_t product_low = product.low;
            uint64_t product_middle2 = product.high;
            uint64_t product_middle1 = lower;
            uint64_t product_high = upper;
            uint64_t product_middle = product_middle1 + product_middle2;
            if (product_middle < product_middle1) {
                product_high++; // overflow carry
            }
            // we want to check whether mantissa *i + i would affect our result
            // This does happen, e.g. with 7.3177701707893310e+15
            if (((product_middle + 1 == 0) && ((product_high & 0x1FF) == 0x1FF) &&
                (product_low + i < product_low))) { // let us be prudent and bail out.
                *success = false;
                return 0;
            }
            upper = product_high;
            lower = product_middle;
        }
        // The final mantissa should be 53 bits with a leading 1.
        // We shift it so that it occupies 54 bits with a leading 1.
        ///////
        uint64_t upperbit = upper >> 63;
        uint64_t mantissa = upper >> (upperbit + 9);
        lz += int(1 ^ upperbit);
        // Here we have mantissa < (1<<54).

        // We have to round to even. The "to even" part
        // is only a problem when we are right in between two floats
        // which we guard against.
        // If we have lots of trailing zeros, we may fall right between two
        // floating-point values.
        if (unlikely((lower == 0) && ((upper & 0x1FF) == 0) &&
            ((mantissa & 3) == 1))) {
            // if mantissa & 1 == 1 we might need to round up.
            //
            // Scenarios:
            // 1. We are not in the middle. Then we should round up.
            //
            // 2. We are right in the middle. Whether we round up depends
            // on the last significant bit: if it is "one" then we round
            // up (round to even) otherwise, we do not.
            //
            // So if the last significant bit is 1, we can safely round up.
            // Hence we only need to bail out if (mantissa & 3) == 1.
            // Otherwise we may need more accuracy or analysis to determine whether
            // we are exactly between two floating-point numbers.
            // It can be triggered with 1e23.
            // Note: because the factor_mantissa and factor_mantissa_low are
            // almost always rounded down (except for small positive powers),
            // almost always should round up.
            * success = false;
            return 0;
        }
        mantissa += mantissa & 1;
        mantissa >>= 1;
        // Here we have mantissa < (1<<53), unless there was an overflow
        if (mantissa >= (1ULL << 53)) {
            //////////
            // This will happen when parsing values such as 7.2057594037927933e+16
            ////////
            mantissa = (1ULL << 52);
            lz--; // undo previous addition
        }
        mantissa &= ~(1ULL << 52);
        uint64_t real_exponent = c.exp - lz;
        // we have to check that real_exponent is in range, otherwise we bail out
        if (unlikely((real_exponent < 1) || (real_exponent > 2046))) {
            *success = false;
            return 0;
        }
        mantissa |= real_exponent << 52;
        mantissa |= (((uint64_t)negative) << 63);
        double d;
        memcpy(&d, &mantissa, sizeof(d));
        *success = true;
        return d;
    }

    really_inline
    static result_type parse_float_strtod(const char*& ptr, double* outDouble, const char* pend) {
        char* endptr = (char*)pend;
        *outDouble = strtod(ptr, &endptr);
        // Some libraries will set errno = ERANGE when the value is subnormal,
        // yet we may want to be able to parse subnormal values.
        // However, we do not want to tolerate NAN or infinite values.
        // There isno realistic application where you might need values so large than
        // they can't fit in binary64. The maximal value is about  1.7976931348623157
        // × 10^308 It is an unimaginable large number. There will never be any piece
        // of engineering involving as many as 10^308 parts. It is estimated that
        // there are about 10^80 atoms in the universe. The estimate for the total
        // number of electrons is similar. Using a double-precision floating-point
        // value, we can represent easily the number of atoms in the universe. We
        // could  also represent the number of ways you can pick any three individual
        // atoms at random in the universe.
        if ((endptr == ptr) || (!std::isfinite(*outDouble))) {
            return result_type::Invalid;
        }
        ptr = endptr;
        return result_type::Double;
    }

    really_inline
    static result_type parse_float_strtod(const wchar_t*& ptr, double* outDouble, const wchar_t* pend) {
        wchar_t* endptr = (wchar_t*)pend;
        *outDouble = wcstod(ptr, &endptr);
        // Some libraries will set errno = ERANGE when the value is subnormal,
        // yet we may want to be able to parse subnormal values.
        // However, we do not want to tolerate NAN or infinite values.
        // There isno realistic application where you might need values so large than
        // they can't fit in binary64. The maximal value is about  1.7976931348623157
        // × 10^308 It is an unimaginable large number. There will never be any piece
        // of engineering involving as many as 10^308 parts. It is estimated that
        // there are about 10^80 atoms in the universe. The estimate for the total
        // number of electrons is similar. Using a double-precision floating-point
        // value, we can represent easily the number of atoms in the universe. We
        // could  also represent the number of ways you can pick any three individual
        // atoms at random in the universe.
        if ((endptr == ptr) || (!std::isfinite(*outDouble))) {
            return result_type::Invalid;
        }
        ptr = endptr;
        return result_type::Double;
    }

#if ( __cplusplus < 201703L )
    template <class _Char, _Char First, _Char... Rest>
    struct one_of_impl
    {
        really_inline static bool call(_Char v)
        {
            return First == v || one_of_impl<_Char, Rest...>::call(v);
        }
    };
    template<class _Char, _Char First>
    struct one_of_impl<_Char, First>
    {
        really_inline static bool call(_Char v)
        {
            return First == v;
        }
    };
    template <class _Char, _Char... Values>
    really_inline bool is_one_of(_Char v)
    {
        return one_of_impl<_Char, Values...>::call(v);
    }
#else
    template <class _Char, _Char... Values>
    really_inline bool is_one_of(_Char v)
    {
        return ((v == Values) || ...);
    }
#endif


    // parse the number at p
    template <class _Char, _Char... DecSeparators>
    WARN_UNUSED
    result_type parse_number_base(const _Char*& pinit, double* outDouble, const _Char* pend) {
        typedef typename std::make_unsigned<_Char>::type _Uchar;
        const _Char* p = pinit;
        bool found_minus = (*p == '-');
        bool negative = false;
        bool is_double = false;
        bool is_overflow = false;

        if (found_minus) {
            ++p;
            negative = true;
            if (p >= pend || (!is_integer(*p) && !is_one_of<_Char, DecSeparators...>(*p))) { // a negative sign must be followed by an integer
                pinit = p;
                return result_type::Invalid;
            }
        }
        else if (*p == '+') {
            ++p;
            if (p >= pend || (!is_integer(*p) && !is_one_of<_Char, DecSeparators...>(*p))) { // a negative sign must be followed by an integer
                pinit = p;
                return result_type::Invalid;
            }
        }

        const _Char* const start_digits = p;

        uint64_t i;      // an unsigned int avoids signed overflows (which are bad)
        if (*p == '0') { // 0 cannot be followed by an integer
            ++p;
            if (p < pend && is_integer(*p)) {
                pinit = p;
                return result_type::Invalid;
            }
            i = 0;
        }
        else if (is_one_of<_Char, DecSeparators...>(*p)) {
            if (p + 1 >= pend || !is_integer(p[1])) { //Can not be an isolated dot
                pinit = p;
                return result_type::Invalid;
            }
            i = 0;
        }
        else {
            if (!(is_integer(*p))) { // must start with an integer
                pinit = p;
                return result_type::Invalid;
            }
            _Uchar digit = *p - '0';
            i = digit;
            p++;
            // the is_made_of_eight_digits_fast routine is unlikely to help here because
            // we rarely see large integer parts like 123456789
            while (p < pend && is_integer(*p)) {
                digit = *p - '0';
                ++p;
                // a multiplication by 10 is cheaper than an arbitrary integer
                // multiplication

                //int64_t::max=2^63-1
                //2 ^ 63 = 9223372036854775808
                //2 ^ 62 = 922337203685477580 = 0x0CCCCCCCCCCCCCCC
                if (unlikely(i >= 922337203685477580))
                {
                    if (i != 922337203685477580 || digit >= 8)
                    {
                        is_overflow = true;
                        while (p < pend && is_integer(*p)) ++p;
                        break;
                    }
                }
                i = 10 * i + digit; // might overflow, we will handle the overflow later
            }
        }

        int64_t exponent = 0;
        const _Char* first_after_period = NULL;
        if (is_one_of<_Char, DecSeparators...>(*p)) {
            is_double = true;
            ++p;
            first_after_period = p;
            if (p < pend && is_integer(*p)) {
                _Uchar digit = *p - '0';
                ++p;
                i = i * 10 + digit; // might overflow + multiplication by 10 is likely
                                    // cheaper than arbitrary mult.
                // we will handle the overflow later
            }
            else {
                pinit = p;
                return result_type::Invalid;
            }
            while (p < pend && is_integer(*p)) {
                _Uchar digit = *p - '0';
                ++p;

                //int64_t::max=2^63-1
                //2 ^ 63 = 9223372036854775808
                //2 ^ 62 = 922337203685477580 = 0x0CCCCCCCCCCCCCCC
                if (unlikely(i >= 922337203685477580))
                {
                    if (i != 922337203685477580 || digit >= 8)
                    {
                        is_overflow = true;
                        while (p < pend && is_integer(*p)) ++p;
                        break;
                    }
                }
                i = i * 10 + digit; // in rare cases, this will overflow, but that's ok
                                    // because we have parse_highprecision_float later.
            }
            exponent = first_after_period - p;
        }

        int digit_count = int(p - start_digits - 1); // used later to guard against overflows
        int64_t exp_number = 0;   // exponential part
        if ((*p | 32) == 'e') {
            is_double = true;
            ++p;
            bool neg_exp = false;
            if ('-' == *p) {
                neg_exp = true;
                ++p;
            }
            else if ('+' == *p) {
                ++p;
            }
            if (p >= pend || !is_integer(*p)) {
                pinit = p;
                return result_type::Invalid;
            }
            _Uchar digit = *p - '0';
            exp_number = digit;
            p++;
            if (p < pend && is_integer(*p)) {
                digit = *p - '0';
                exp_number = 10 * exp_number + digit;
                ++p;
            }
            if (p < pend && is_integer(*p)) {
                digit = *p - '0';
                exp_number = 10 * exp_number + digit;
                ++p;
            }
            while (p < pend && is_integer(*p)) {
                if (exp_number > 0x100000000) { // we need to check for overflows
                                                // we refuse to parse this
                    pinit = p;
                    return result_type::Invalid;
                }
                digit = *p - '0';
                exp_number = 10 * exp_number + digit;
                ++p;
            }
            exponent += (neg_exp ? -exp_number : exp_number);
        }

        if (unlikely(is_overflow)) {
            return parse_float_strtod(pinit, outDouble, pend);
        }

        // If we frequently had to deal with long strings of digits,
        // we could extend our code by using a 128-bit integer instead
        // of a 64-bit integer. However, this is uncommon.
        if (is_double && unlikely(digit_count >= 19)) { // this is uncommon
          // It is possible that the integer had an overflow.
          // We have to handle the case where we have 0.0000somenumber.
            const _Char* start = start_digits;
            while (*start == '0' || is_one_of<_Char, DecSeparators...>(*start)) {
                start++;
            }

            // we over-decrement by one when there is a decimal separator
            digit_count -= int(start - start_digits);
            if (digit_count >= 19) {
                // Chances are good that we had an overflow!
                // We start anew.
                // This will happen in the following examples:
                // 10000000000000000000000000000000000000000000e+308
                // 3.1415926535897932384626433832795028841971693993751
                //
                return parse_float_strtod(pinit, outDouble, pend);
            }
        }
        assert(!is_overflow);

        if (is_double)
        {
            if (unlikely(exponent < FASTFLOAT_SMALLEST_POWER) || (exponent > FASTFLOAT_LARGEST_POWER)) {
                // this is almost never going to get called!!!
                // exponent could be as low as 325
                return parse_float_strtod(pinit, outDouble, pend);
            }
            // from this point forward, exponent >= FASTFLOAT_SMALLEST_POWER and
            // exponent <= FASTFLOAT_LARGEST_POWER
            bool success = true;
            *outDouble = compute_float_64(exponent, i, negative, &success);
            if (!success) {
                // we are almost never going to get here.
                return parse_float_strtod(pinit, outDouble, pend);
            }
            pinit = p;
            return result_type::Double;
        }
        else
        {
            *reinterpret_cast<int64_t*>(outDouble) = negative ? -(int64_t)i : (int64_t)i;
            pinit = p;
            return result_type::Long;
        }
    }

} // namespace fast_double_parser

#endif

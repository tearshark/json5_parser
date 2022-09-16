
namespace
{
	#define _MM_SHUFFLE8(fp7, fp6, fp5, fp4, fp3, fp2, fp1, fp0) \
		(((fp7) << 21) | ((fp6) << 18) | ((fp5) << 15) | ((fp4) << 12)) | \
		(((fp3) << 9) | ((fp2) << 6) | ((fp1) << 3) | ((fp0)))

	__m128i _mm_shuffle_epi16(__m128i _A, int _Imm)
	{
		_Imm &= 0xffffff;
		char m01 = (_Imm >> 0) & 0x7, m03 = (_Imm >> 3) & 0x7;
		char m05 = (_Imm >> 6) & 0x7, m07 = (_Imm >> 9) & 0x7;
		char m09 = (_Imm >> 12) & 0x7, m11 = (_Imm >> 15) & 0x7;
		char m13 = (_Imm >> 18) & 0x7, m15 = (_Imm >> 21) & 0x7;
		m01 <<= 1; m03 <<= 1; m05 <<= 1; m07 <<= 1;
		m09 <<= 1; m11 <<= 1; m13 <<= 1; m15 <<= 1;
		char m00 = m01 + 1, m02 = m03 + 1, m04 = m05 + 1, m06 = m07 + 1;
		char m08 = m09 + 1, m10 = m11 + 1, m12 = m13 + 1, m14 = m15 + 1;

		//__m128i vMask = _mm_set_epi8(m00, m01, m02, m03, m04, m05, m06, m07, m08, m09, m10, m11, m12, m13, m14, m15);
		__m128i vMask = _mm_set_epi8(m14, m15, m12, m13, m10, m11, m08, m09, m06, m07, m04, m05, m02, m03, m00, m01);
		return _mm_shuffle_epi8(_A, vMask);
	}

	//a��b����4��16λ������(���Ը�64λ)
	//r[0..7] = a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]
	inline __m128i x_mm_dotp_i16x4(__m128i i16x8_a, __m128i i16x8_b) noexcept
	{
		__m128i i32x4 = _mm_madd_epi16(i16x8_a, i16x8_b);
		i32x4 = _mm_add_epi32(i32x4, _mm_shuffle_epi32(i32x4, _MM_SHUFFLE(2, 3, 0, 1)));
		return i32x4;
	}

	//a��b����8��16λ������
	//r[0..7] = a[0]*b[0]+a[1]*b[1]+a[2]*b[2]+a[3]*b[3]+a[4]*b[4]+a[5]*b[5]+a[6]*b[6]+a[7]*b[7]
	inline __m128i x_mm_dotp_i16x8(__m128i i16x8_a, __m128i i16x8_b) noexcept
	{
		__m128i i32x4 = _mm_madd_epi16(i16x8_a, i16x8_b);
		i32x4 = _mm_add_epi32(i32x4, _mm_shuffle_epi32(i32x4, _MM_SHUFFLE(2, 3, 0, 1)));
		i32x4 = _mm_add_epi32(i32x4, _mm_shuffle_epi32(i32x4, _MM_SHUFFLE(0, 1, 2, 3)));
		return i32x4;
	}

	//��8��16λ��ĸ��ת����һ�����99999999������
	//r = ((i16x8[0]-'0')*1 + (i16x8[1]-'0')*10 + (i16x8[2]-'0')*100 + (i16x8[3]-'0')*1000)+
	//    ((i16x8[4]-'0')*1 + (i16x8[5]-'0')*10 + (i16x8[6]-'0')*100 + (i16x8[7]-'0')*1000)*1000
	static int x_mm_cvt_i16x8_i32(__m128i i16x8) noexcept
	{
		//���8��16λ�����֣���������ĸ��
		i16x8 = _mm_sub_epi16(i16x8, _mm_set1_epi16('0'));

		//���ڷ�Χ���ƣ�ֻ�����ɵ�4������ת��һ��������4������ת��һ����
		__m128i i32x4 = _mm_madd_epi16(i16x8, _mm_set_epi16(1000, 100, 10, 1, 1000, 100, 10, 1));
		i32x4 = _mm_add_epi32(i32x4, _mm_shuffle_epi32(i32x4, _MM_SHUFFLE(2, 3, 0, 1)));
		//���ڣ�i32x4[0]=i32x4[1]=ʮ���Ƶĸ�4λ���֣�i32x4[2]=i32x4[3]=ʮ���Ƶĵ�4λ����
		
		//����֮ǰʮ���Ƶĸ�4λ�����ٳ˵�10000
		i32x4 = _mm_mul_epu32(i32x4, _mm_set_epi32(0, 10000, 0, 1));
		//��ʮ���Ƶĸ�4λ*10000��Ľ�������ϵ�4λ
		i32x4 = _mm_add_epi32(i32x4, _mm_shuffle_epi32(i32x4, _MM_SHUFFLE(3, 0, 1, 2)));

		//����ڵ�64λ���á����ڷ�Χû����32λ����ֱ��ȡ��32λ���ǽ��
		int val = _mm_cvtsi128_si32(i32x4);
		return val;
	}

	//��4��16λ��ĸ��ת����һ�����9999������
	//r = ((i16x8[0]-'0')*1 + (i16x8[1]-'0')*10 + (i16x8[2]-'0')*100 + (i16x8[3]-'0')*1000)
	static int x_mm_cvt_i16x4_i32(__m128i i16x4) noexcept
	{
		//���8��16λ�����֣���������ĸ��
		i16x4 = _mm_sub_epi16(i16x4, _mm_set1_epi16('0'));
		__m128i i32x4 = x_mm_dotp_i16x4(i16x4, _mm_set_epi16(0, 0, 0, 0, 1000, 100, 10, 1));
		int val = _mm_cvtsi128_si32(i32x4);
		return val;
	}

	//�ж�x�ǲ���һ��������ĸ([0,9]֮�����ĸ)
#ifndef x_is_digit
	inline bool x_is_digit(int32_t x) noexcept
	{
		return ((x - '0') | ('9' - x)) >= 0;
	}
#endif



	//wchar_t�ڲ�ͬ��ƽ̨�ϣ��ǲ�ͬ�ġ�16λ����32λ������ϸ�ڻ᲻ͬ
	//�ʲ��ܼ򵥵����char/wchar_t�����ػ�
	template<size_t _CharSize>
	struct x_convert_char_selector;

	template<>
	struct x_convert_char_selector<2>
	{
		using type = char16_t;

		static __m128i load_xdigitx4(const type* s) noexcept
		{
#if defined(_M_X64) || defined(__x86_64__)
			return _mm_cvtsi64_si128(*(int64_t*)s);
#elif defined(_M_IX86) || defined(__i386__)
			const int32_t* v = (const int32_t*)(s);
			return _mm_set_epi32(0, 0, v[1], v[0]);
#else
#error "Unknown platform"
#endif
		}

		//����8��������ĸ
		//����i16x8��128λֵ
		static __m128i load_xdigitx8(const type* s) noexcept
		{
			return _mm_load_si128((const __m128i*)s);
		}

		//i16x4��8��16λ��ĸ
		//��� i16x4[0], i16x4[1], i16x4[2], i16x4[3] �ĸ����ǲ��Ƕ���['0', '9']֮���������ĸ
		//����i8x4[4...7]
		//���ڴ���unicode 16bits
		//r.bits[0,1] = is_digit(i16x4[0])
		//r.bits[2,3] = is_digit(i16x4[1])
		//r.bits[4,5] = is_digit(i16x4[2])
		//r.bits[6,7] = is_digit(i16x4[3])
		static int digit_mask(__m128i i16x4) noexcept
		{
			__m128i i16x8_gt = _mm_cmpgt_epi16(i16x4, _mm_set_epi16(0, 0, 0, 0, '0' - 1, '0' - 1, '0' - 1, '0' - 1));
			__m128i i16x8_lt = _mm_cmplt_epi16(i16x4, _mm_set_epi16(0, 0, 0, 0, '9' + 1, '9' + 1, '9' + 1, '9' + 1));
			__m128i i16x8_and = _mm_and_si128(i16x8_gt, i16x8_lt);
			int i8x4_mask = _mm_movemask_epi8(i16x8_and);
			return i8x4_mask;
		}

		//��4��16λ��ĸ(���Ը�4��16λ��ĸ����ת����һ�����9999������
		//i16x4_mul��4��16λ����(��4��16λ���ֱ����趨Ϊ0)��������Ҫ�趨��ת�����ٸ���ĸ:
		//	4��ĸ : _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000)
		//	3��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100)
		//	2��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 1, 10)
		//	1��ĸ : _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 1)
		//r = (i16x4[0]-'0')*i16x4_mul[0] + (i16x4[1]-'0')*i16x4_mul[1] + (i16x4[2]-'0')*i16x4_mul[2] + (i16x4[3]-'0')*i16x4_mul[3]
		static int cvt_xcharx4_i32(__m128i i16x4, __m128i i16x4_mul) noexcept
		{
			i16x4 = _mm_sub_epi16(i16x4, _mm_set1_epi16('0'));

			__m128i i32x4 = x_mm_dotp_i16x4(i16x4, i16x4_mul);
			int val = _mm_cvtsi128_si32(i32x4) & 0xffff;
			return val;
		}

		enum
		{
			MASK4 = 0xff,
			MASK3 = 0x3f,
			MASK2 = 0x0f,
			MASK1 = 0x03,
		};
	};

	template<>
	struct x_convert_char_selector<1>
	{
		using type = char;

		static __m128i load_xdigitx4(const type* s) noexcept
		{
			__m128i i8x8 = _mm_cvtsi32_si128(*(int*)s);
			return _mm_unpacklo_epi8(i8x8, _mm_setzero_si128());
		}

		//����8��������ĸ
		//����i16x8��128λֵ
		static __m128i load_xdigitx8(const type* s) noexcept
		{
			__m128i i8x8;
#if defined(_M_X64) || defined(__x86_64__)
			i8x8 = _mm_cvtsi64_si128(*(int64_t*)s);
#elif defined(_M_IX86) || defined(__i386__)
			const int32_t* v = (const int32_t*)(s);
			i8x8 = _mm_set_epi32(0, 0, v[1], v[0]);
#else
#error "Unknown platform"
#endif
			return _mm_unpacklo_epi8(i8x8, _mm_setzero_si128());
		}
	};

	template<>
	struct x_convert_char_selector<4>
	{
		using type = char32_t;

		static __m128i load_xdigitx4(const type* s) noexcept
		{
			__m128i lo = _mm_load_si128((__m128i*)s);
			return _mm_packs_epi32(lo, _mm_setzero_si128());
		}

		static __m128i load_xdigitx8(const type* s) noexcept
		{
			__m128i lo = _mm_load_si128((__m128i*)s);
			__m128i hi = _mm_load_si128(((__m128i*)s) + 1);
			return _mm_packs_epi32(lo, hi);
		}
	};

	//��8�������ַ�ת��λ��������"12345678"ת������12345678��
	template<class type>
	int64_t x_mm_convert_digitx8_long(int64_t result, const type* s) noexcept
	{
		using traits_t = x_convert_char_selector<sizeof(type)>;
	
		__m128i i16x8 = traits_t::load_xdigitx8((typename traits_t::type*)s);
		int val = x_mm_cvt_i16x8_i32(i16x8);
		return val;
	}

	//��4�������ַ�ת��λ��������"1234"ת������1234��
	template<class type>
	int64_t x_mm_convert_digitx4_long(int64_t result, const type* s) noexcept
	{
		using traits_t = x_convert_char_selector<sizeof(type)>;

		__m128i i16x8 = traits_t::load_xdigitx4((typename traits_t::type*)s);
		int val = x_mm_cvt_i16x4_i32(i16x8);
		return val;
	}
	//���ַ���ת��Ϊ����
	//�������������ַ������߳���int64_t�ɱ��ķ�Χ����ֹͣ
	//psz:��Ҫת�����ַ���
	//overflow:�������������������Ϊtrue�����򣬲���ı�overflow��ֵ
	//����ֵ:�Ѿ�ת��������
	template<class type>
	int64_t x_mm_convert_string_long(int64_t result, const type*& psz, const type* e, bool& overflow) noexcept
	{
		using traits_t = x_convert_char_selector<sizeof(type)>;
		using traits16_t = x_convert_char_selector<2>;

		constexpr int64_t MAX_LONG = (std::numeric_limits<int64_t>::max)();
		constexpr int64_t LIMIT_LONG_9999 = (MAX_LONG - 9999) / 10000;
		constexpr int64_t LIMIT_LONG_999 = (MAX_LONG - 999) / 1000;
		constexpr int64_t LIMIT_LONG_99 = (MAX_LONG - 99) / 100;
		constexpr int64_t LIMIT_LONG_9 = (MAX_LONG - 9) / 10;

		const type* s = psz;
		for (; s < e; )
		{
			auto remaind = e - s;

			__m128i i16x4 = traits_t::load_xdigitx4(s);
			int mask = traits16_t::digit_mask(i16x4);
			if (mask == traits16_t::MASK4 && remaind >= 4)
			{
				if (result > LIMIT_LONG_9999) break;

				int val = traits16_t::cvt_xcharx4_i32(i16x4, _mm_set_epi16(0, 0, 0, 0, 1, 10, 100, 1000));
				result = result * 10000 + val;
				s += 4;
			}
			else if ((mask & traits16_t::MASK3) == traits16_t::MASK3 && remaind >= 3)
			{
				if (result > LIMIT_LONG_999) break;

				int val = traits16_t::cvt_xcharx4_i32(i16x4, _mm_set_epi16(0, 0, 0, 0, 0, 1, 10, 100));
				result = result * 1000 + val;

				psz = s + 3;
				return result;
			}
			else if ((mask & traits16_t::MASK2) == traits16_t::MASK2 && remaind >= 2)
			{
				if (result > LIMIT_LONG_99) break;

				//int val = traits16_t::cvt_xcharx4_i32(i8x4, _mm_set_epi16(0, 0, 0, 0, 0, 0, 1, 10));
				int val = (s[0] - (type)'0') * 10 + (s[1] - (type)'0');
				result = result * 100 + val;

				psz = s + 2;
				return result;
			}
			else if ((mask & traits16_t::MASK1) == traits16_t::MASK1)
			{
				if (result > LIMIT_LONG_9) break;

				int val = *s - (type)'0';
				result = result * 10 + val;

				psz = s + 1;
				return result;
			}
			else
			{
				psz = s;
				return result;
			}
		}

		for (; s < e && x_is_digit(*s); ++s)
		{
			if (result >= 0x0CCCCCCCCCCCCCCCULL)	// 2^63 = 9223372036854775808
			{
				if (result != 0x0CCCCCCCCCCCCCCCULL || *s >= (type)'8')
				{
					overflow = true;
					break;
				}
			}

			int val = *s - (type)'0';
			result = result * 10 + val;
		}

		psz = s;
		return result;
	}

	static const double DOUBLE_E[] =
	{ // 1e-0...1e308: 309 * 8 bytes = 2472 bytes
		1e+0,
		1e+1,  1e+2,  1e+3,  1e+4,  1e+5,  1e+6,  1e+7,  1e+8,  1e+9,  1e+10, 1e+11, 1e+12, 1e+13, 1e+14, 1e+15, 1e+16, 1e+17, 1e+18, 1e+19, 1e+20,
		1e+21, 1e+22, 1e+23, 1e+24, 1e+25, 1e+26, 1e+27, 1e+28, 1e+29, 1e+30, 1e+31, 1e+32, 1e+33, 1e+34, 1e+35, 1e+36, 1e+37, 1e+38, 1e+39, 1e+40,
		1e+41, 1e+42, 1e+43, 1e+44, 1e+45, 1e+46, 1e+47, 1e+48, 1e+49, 1e+50, 1e+51, 1e+52, 1e+53, 1e+54, 1e+55, 1e+56, 1e+57, 1e+58, 1e+59, 1e+60,
		1e+61, 1e+62, 1e+63, 1e+64, 1e+65, 1e+66, 1e+67, 1e+68, 1e+69, 1e+70, 1e+71, 1e+72, 1e+73, 1e+74, 1e+75, 1e+76, 1e+77, 1e+78, 1e+79, 1e+80,
		1e+81, 1e+82, 1e+83, 1e+84, 1e+85, 1e+86, 1e+87, 1e+88, 1e+89, 1e+90, 1e+91, 1e+92, 1e+93, 1e+94, 1e+95, 1e+96, 1e+97, 1e+98, 1e+99, 1e+100,
		1e+101,1e+102,1e+103,1e+104,1e+105,1e+106,1e+107,1e+108,1e+109,1e+110,1e+111,1e+112,1e+113,1e+114,1e+115,1e+116,1e+117,1e+118,1e+119,1e+120,
		1e+121,1e+122,1e+123,1e+124,1e+125,1e+126,1e+127,1e+128,1e+129,1e+130,1e+131,1e+132,1e+133,1e+134,1e+135,1e+136,1e+137,1e+138,1e+139,1e+140,
		1e+141,1e+142,1e+143,1e+144,1e+145,1e+146,1e+147,1e+148,1e+149,1e+150,1e+151,1e+152,1e+153,1e+154,1e+155,1e+156,1e+157,1e+158,1e+159,1e+160,
		1e+161,1e+162,1e+163,1e+164,1e+165,1e+166,1e+167,1e+168,1e+169,1e+170,1e+171,1e+172,1e+173,1e+174,1e+175,1e+176,1e+177,1e+178,1e+179,1e+180,
		1e+181,1e+182,1e+183,1e+184,1e+185,1e+186,1e+187,1e+188,1e+189,1e+190,1e+191,1e+192,1e+193,1e+194,1e+195,1e+196,1e+197,1e+198,1e+199,1e+200,
		1e+201,1e+202,1e+203,1e+204,1e+205,1e+206,1e+207,1e+208,1e+209,1e+210,1e+211,1e+212,1e+213,1e+214,1e+215,1e+216,1e+217,1e+218,1e+219,1e+220,
		1e+221,1e+222,1e+223,1e+224,1e+225,1e+226,1e+227,1e+228,1e+229,1e+230,1e+231,1e+232,1e+233,1e+234,1e+235,1e+236,1e+237,1e+238,1e+239,1e+240,
		1e+241,1e+242,1e+243,1e+244,1e+245,1e+246,1e+247,1e+248,1e+249,1e+250,1e+251,1e+252,1e+253,1e+254,1e+255,1e+256,1e+257,1e+258,1e+259,1e+260,
		1e+261,1e+262,1e+263,1e+264,1e+265,1e+266,1e+267,1e+268,1e+269,1e+270,1e+271,1e+272,1e+273,1e+274,1e+275,1e+276,1e+277,1e+278,1e+279,1e+280,
		1e+281,1e+282,1e+283,1e+284,1e+285,1e+286,1e+287,1e+288,1e+289,1e+290,1e+291,1e+292,1e+293,1e+294,1e+295,1e+296,1e+297,1e+298,1e+299,1e+300,
		1e+301,1e+302,1e+303,1e+304,1e+305,1e+306,1e+307,1e+308
	};

	static const double DOUBLE_NE[] =
	{ // 1e-0...1e308: 309 * 8 bytes = 2472 bytes
		1e-0,
		1e-1,  1e-2,  1e-3,  1e-4,  1e-5,  1e-6,  1e-7,  1e-8,  1e-9,  1e-10, 1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, 1e-18, 1e-19, 1e-20,
		1e-21, 1e-22, 1e-23, 1e-24, 1e-25, 1e-26, 1e-27, 1e-28, 1e-29, 1e-30, 1e-31, 1e-32, 1e-33, 1e-34, 1e-35, 1e-36, 1e-37, 1e-38, 1e-39, 1e-40,
		1e-41, 1e-42, 1e-43, 1e-44, 1e-45, 1e-46, 1e-47, 1e-48, 1e-49, 1e-50, 1e-51, 1e-52, 1e-53, 1e-54, 1e-55, 1e-56, 1e-57, 1e-58, 1e-59, 1e-60,
		1e-61, 1e-62, 1e-63, 1e-64, 1e-65, 1e-66, 1e-67, 1e-68, 1e-69, 1e-70, 1e-71, 1e-72, 1e-73, 1e-74, 1e-75, 1e-76, 1e-77, 1e-78, 1e-79, 1e-80,
		1e-81, 1e-82, 1e-83, 1e-84, 1e-85, 1e-86, 1e-87, 1e-88, 1e-89, 1e-90, 1e-91, 1e-92, 1e-93, 1e-94, 1e-95, 1e-96, 1e-97, 1e-98, 1e-99, 1e-100,
		1e-101,1e-102,1e-103,1e-104,1e-105,1e-106,1e-107,1e-108,1e-109,1e-110,1e-111,1e-112,1e-113,1e-114,1e-115,1e-116,1e-117,1e-118,1e-119,1e-120,
		1e-121,1e-122,1e-123,1e-124,1e-125,1e-126,1e-127,1e-128,1e-129,1e-130,1e-131,1e-132,1e-133,1e-134,1e-135,1e-136,1e-137,1e-138,1e-139,1e-140,
		1e-141,1e-142,1e-143,1e-144,1e-145,1e-146,1e-147,1e-148,1e-149,1e-150,1e-151,1e-152,1e-153,1e-154,1e-155,1e-156,1e-157,1e-158,1e-159,1e-160,
		1e-161,1e-162,1e-163,1e-164,1e-165,1e-166,1e-167,1e-168,1e-169,1e-170,1e-171,1e-172,1e-173,1e-174,1e-175,1e-176,1e-177,1e-178,1e-179,1e-180,
		1e-181,1e-182,1e-183,1e-184,1e-185,1e-186,1e-187,1e-188,1e-189,1e-190,1e-191,1e-192,1e-193,1e-194,1e-195,1e-196,1e-197,1e-198,1e-199,1e-200,
		1e-201,1e-202,1e-203,1e-204,1e-205,1e-206,1e-207,1e-208,1e-209,1e-210,1e-211,1e-212,1e-213,1e-214,1e-215,1e-216,1e-217,1e-218,1e-219,1e-220,
		1e-221,1e-222,1e-223,1e-224,1e-225,1e-226,1e-227,1e-228,1e-229,1e-230,1e-231,1e-232,1e-233,1e-234,1e-235,1e-236,1e-237,1e-238,1e-239,1e-240,
		1e-241,1e-242,1e-243,1e-244,1e-245,1e-246,1e-247,1e-248,1e-249,1e-250,1e-251,1e-252,1e-253,1e-254,1e-255,1e-256,1e-257,1e-258,1e-259,1e-260,
		1e-261,1e-262,1e-263,1e-264,1e-265,1e-266,1e-267,1e-268,1e-269,1e-270,1e-271,1e-272,1e-273,1e-274,1e-275,1e-276,1e-277,1e-278,1e-279,1e-280,
		1e-281,1e-282,1e-283,1e-284,1e-285,1e-286,1e-287,1e-288,1e-289,1e-290,1e-291,1e-292,1e-293,1e-294,1e-295,1e-296,1e-297,1e-298,1e-299,1e-300,
		1e-301,1e-302,1e-303,1e-304,1e-305,1e-306,1e-307,1e-308
	};

	static const int64_t LONG_E[] =
	{
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000,
		10000000000,
		100000000000,
		1000000000000,
		10000000000000,
		100000000000000,
		1000000000000000,
		10000000000000000,
	};

	inline double x_fast_path(double significand, intptr_t exp) noexcept
	{
		if (exp < -308)
			return -INFINITY;
		else if (exp >= 0)
			return significand * DOUBLE_E[exp];
		else
			return significand * DOUBLE_NE[-exp];
	}

	template<class _CharType>
	std::tuple<number_value, parser_result> simd_double_parser2(const _CharType*& s, const _CharType* const pszEnd) noexcept
	{
		const _CharType* psz = s;

		//�ȴ���������
		bool minus = *psz == '-';
		if (minus) ++psz;
		else if (*psz == '+') ++psz;

		if (psz >= pszEnd)
		{
			number_value nv = { 0 };
			return { nv, parser_result::Invalid };
		}

		intptr_t exp = 0;

		double dval = 0.0f;
		bool useDouble = false;	//��ʼû����������������ˣ�����Ҫʹ�ø������㷨
		int64_t i64 = x_mm_convert_string_long(0, psz, pszEnd, useDouble);
		if (useDouble)
		{//���������ʹ�ø������㷨
			dval = (double)i64;

			const _CharType* const pszSaved = psz;
			for (; x_is_digit(*psz); ++psz);
			exp = psz - pszSaved;
		}

		if (psz < pszEnd && *psz == '.')
		{//����С������
			++psz;
			const _CharType* pszDot = psz;

			if (!useDouble)
			{//��δ���������С������������
				i64 = x_mm_convert_string_long(i64, psz, pszEnd, useDouble);
			}
			dval = (double)i64;

			if (useDouble)
			{
				exp += pszDot - psz;	//����ĸ�������ָ��

				//�Ѿ��ִﾫ�����ޣ������ַ����ٷ���
				for (; x_is_digit(*psz); ++psz);
			}
			else
			{
				exp += pszDot - psz;
				useDouble = true;
			}
		}

		if (psz < pszEnd && (*psz | 32) == 'e')
		{//����ָ��
			if (!useDouble)
			{//���֮ǰ���������������Ҫ���ո��������н�����
				dval = (double)i64;
				useDouble = true;
			}
			++psz;

			bool expMinus = false;
			if (psz < pszEnd && *psz == '+')
			{
				++psz;
			}
			else if (psz < pszEnd && *psz == '-')
			{
				++psz;
				expMinus = true;
			}

			if (psz >= pszEnd)
			{
				number_value nv = { 0 };
				return { nv, parser_result::Invalid };
			}

			bool overflow = false;	//��ʼû����������������ˣ�����Ϊ����һ����Ч����
			int64_t e2 = x_mm_convert_string_long(0, psz, pszEnd, overflow);

			if (overflow || e2 > ((std::numeric_limits<int32_t>::max)() / 2))
			{
				number_value nv = { 0 };
				return { nv, parser_result::Invalid };
			}

			//��ָ���ۻ���ȥ
			if (expMinus)
				exp -= (intptr_t)e2;
			else
				exp += (intptr_t)e2;
		}

		if (useDouble)
		{
			if (exp < -330)	//330 = 308 + 22
			{
				dval = -INFINITY;
			}
			else if (exp < -308)
			{
				dval = x_fast_path(dval, -308);
				dval = x_fast_path(dval, exp + 308);
			}
			else if (exp > 330)	//330 = 308 + 22
			{
				dval = INFINITY;
			}
			else if (exp > 308)
			{
				dval = x_fast_path(dval, 308);
				dval = x_fast_path(dval, exp - 308);
			}
			else if (exp != 0)
			{
				dval = x_fast_path(dval, exp);
			}

			number_value nv;
			nv.d = minus ? -dval : dval;

			s = psz;
			return { nv, parser_result::Double };
		}
		else
		{
			number_value nv;
			nv.l = minus ? -i64 : i64;

			s = psz;
			return { nv, parser_result::Long };
		}
	}

	//ERROR!!!
	template<class _CharType>
	std::tuple<number_value, parser_result> simd_double_parser3(const _CharType*& pszStart, const _CharType* const pszEnd) noexcept
	{
		const _CharType* psz = pszStart;

		//�ȴ���������
		bool minus = *psz == '-';
		if (minus) ++psz;
		else if (*psz == '+') ++psz;

		if (psz >= pszEnd)
		{
			number_value nv = { 0 };
			return { nv, parser_result::Invalid };
		}

		const _CharType* pszDot = nullptr;
		if (*psz == (_CharType)'0' && psz + 1 < pszEnd)
		{
			if (psz[1] != (_CharType)'.')
			{
				number_value nv = { 0 };
				return { nv, parser_result::Invalid };
			}

			pszDot = psz;
			psz += 2;
		}

		__m128i i16x8 = _mm_set1_epi8('0');
		int64_t i64 = 0;
		intptr_t exp = 0;

		const _CharType* pszSaved = psz;
		for (int i = 0; i < 8 && psz < pszEnd; ++i)
		{
			if (*psz == (_CharType)'.')
			{
				if (pszDot == nullptr)
				{
					--exp;
					pszDot = psz;
					++psz;
					if (psz >= pszEnd)
						break;
				}
				else
				{
					number_value nv = { 0 };
					return { nv, parser_result::Invalid };
				}
			}

			if (x_is_digit(*psz))
				i16x8.m128i_u64[0] = (i16x8.m128i_u64[0] << 8) | static_cast<uint8_t>(*psz);
			else
				break;

			++psz;
		}
		
		exp += (psz - pszSaved);
		if (exp <= 0)
		{
			number_value nv = { 0 };
			return { nv, parser_result::Invalid };
		}

		i16x8 = _mm_unpacklo_epi8(i16x8, _mm_setzero_si128());
		if (exp <= 4)
			i64 = x_mm_cvt_i16x4_i32(i16x8);
		else
			i64 = x_mm_cvt_i16x8_i32(i16x8);


		if (exp == 8)
		{
			int dcount = 0;
			pszSaved = psz;

			i16x8 = _mm_set1_epi8('0');
			for (int i = 0; i < 8 && psz < pszEnd; ++i)
			{
				if (*psz == (_CharType)'.')
				{
					if (pszDot == nullptr)
					{
						--dcount;
						pszDot = psz;
						++psz;
						if (psz >= pszEnd)
							break;
					}
					else
					{
						number_value nv = { 0 };
						return { nv, parser_result::Invalid };
					}
				}

				if (x_is_digit(*psz))
					i16x8.m128i_u64[0] = (i16x8.m128i_u64[0] << 8) | static_cast<uint8_t>(*psz);
				else
					break;

				++psz;
			}

			dcount += (psz - pszSaved);
			if (dcount > 0)
			{
				exp += dcount;

				i16x8 = _mm_unpacklo_epi8(i16x8, _mm_setzero_si128());
				if (dcount <= 4)
					i64 = i64 * LONG_E[dcount] + x_mm_cvt_i16x4_i32(i16x8);
				else
					i64 = i64 * LONG_E[dcount] + x_mm_cvt_i16x8_i32(i16x8);
			}
		}

		bool useDouble = false;
		if (exp == 16)
		{
			for (; psz < pszEnd && x_is_digit(*psz); ++psz)
			{
				if (i64 >= 0x0CCCCCCCCCCCCCCCULL)	// 2^63 = 9223372036854775808
				{
					if (i64 != 0x0CCCCCCCCCCCCCCCULL || *psz >= (_CharType)'8')
					{
						useDouble = true;
						break;
					}
				}

				int val = *psz - (_CharType)'0';
				i64 = i64 * 10 + val;
			}
		}

		double dval = 0.0;
		if (useDouble)
		{
			dval = (double)i64;

			if (pszDot == nullptr)
			{
				pszSaved = psz;
				for (; psz < pszEnd && x_is_digit(*psz); ++psz);
				exp = psz - pszSaved;

				if (*psz == '.')
				{
					++psz;
					for (; psz < pszEnd && x_is_digit(*psz); ++psz);
				}
			}
			else
			{
				exp = pszDot - psz;
				for (; psz < pszEnd && x_is_digit(*psz); ++psz);
			}
		}
		else
		{
			if (pszDot != nullptr)
			{
				dval = (double)i64;
				useDouble = true;

				exp -= psz - pszDot;
			}
			else
			{
				exp = 0;
			}
		}

		if (psz < pszEnd && (*psz | 32) == 'e')
		{//����ָ��
			if (!useDouble)
			{//���֮ǰ���������������Ҫ���ո��������н�����
				dval = (double)i64;
				useDouble = true;
			}
			++psz;

			bool expMinus = false;
			if (psz < pszEnd && *psz == '+')
			{
				++psz;
			}
			else if (psz < pszEnd && *psz == '-')
			{
				++psz;
				expMinus = true;
			}

			if (psz >= pszEnd)
			{
				number_value nv = { 0 };
				return { nv, parser_result::Invalid };
			}

			bool overflow = false;	//��ʼû����������������ˣ�����Ϊ����һ����Ч����
			int64_t e2 = x_mm_convert_string_long(0, psz, pszEnd, overflow);

			if (overflow || e2 > ((std::numeric_limits<int32_t>::max)() / 2))
			{
				number_value nv = { 0 };
				return { nv, parser_result::Invalid };
			}

			//��ָ���ۻ���ȥ
			if (expMinus)
				exp -= (intptr_t)e2;
			else
				exp += (intptr_t)e2;
		}

		if (useDouble)
		{
			if (exp < -330)	//330 = 308 + 22
			{
				dval = -INFINITY;
			}
			else if (exp < -308)
			{
				dval = x_fast_path(dval, -308);
				dval = x_fast_path(dval, exp + 308);
			}
			else if (exp > 330)	//330 = 308 + 22
			{
				dval = INFINITY;
			}
			else if (exp > 308)
			{
				dval = x_fast_path(dval, 308);
				dval = x_fast_path(dval, exp - 308);
			}
			else if (exp != 0)
			{
				dval = x_fast_path(dval, exp);
			}

			number_value nv;
			nv.d = minus ? -dval : dval;

			pszStart = psz;
			return { nv, parser_result::Double };
		}
		else
		{
			number_value nv;
			nv.l = minus ? -i64 : i64;

			pszStart = psz;
			return { nv, parser_result::Long };
		}
	}
}

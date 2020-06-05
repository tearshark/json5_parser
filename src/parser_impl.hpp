//2018-01-09  支持 JSON5 规范

JSON_Alloctor::JSON_Alloctor(size_t nNumBatch/* = 4*/)
	: m_pNextAlloc(nullptr)
	, m_pEndAlloc(nullptr)
	, m_nAllocedCount(0)
	, m_pFirstNode(nullptr)
	, m_nNumBatch((std::max)(nNumBatch, (size_t)64))
{
}

JSON_Alloctor::~JSON_Alloctor()
{
	clear(0);
}

JSON_Value * JSON_Alloctor::alloc()
{
	if (m_pNextAlloc >= m_pEndAlloc)
	{
		size_t nNumAlloc = sizeof(JSON_Value) * m_nNumBatch;

		char * pNode = (char *)_aligned_malloc(nNumAlloc, alignof(JSON_Value));
		//memset(pNode, 0, nNumAlloc);

		*(char **)pNode = m_pFirstNode;
		m_pFirstNode = pNode;

		m_pNextAlloc = (JSON_Value *)(pNode + sizeof(JSON_Value));
		m_pEndAlloc = m_pNextAlloc + m_nNumBatch - 1;
	}

	JSON_Value * t = m_pNextAlloc;
	++m_pNextAlloc;
	++m_nAllocedCount;

	return t;
}

void JSON_Alloctor::clear(size_t nNumBatch)
{
	for (char * p = m_pFirstNode; p; )
	{
		char * t = *(char **)p;
		_aligned_free(p);
		p = t;
	}
	m_pNextAlloc = nullptr;
	m_pEndAlloc = nullptr;
	m_nAllocedCount = 0;
	m_pFirstNode = nullptr;
	m_nNumBatch = (std::max)(nNumBatch, (size_t)64);
}


JSON_Parser::JSON_Parser()
	:m_pRootValue(nullptr)
{
}

JSON_Parser::~JSON_Parser()
{

}

static inline size_t _json_is_flag(int c, size_t f) { return __json_char_flag[(unsigned char)c] & f; }
static inline  size_t _json_is_space(int c) { JSON_U(c, 0);return _json_is_flag(c, jsoncf_space); }
static inline  size_t _json_is_name(int c) { JSON_U(c, 1);return _json_is_flag(c, jsoncf_name); }
static inline  size_t _json_is_digit(int c) { JSON_U(c, 0);return _json_is_flag(c, jsoncf_digit); }
static inline  size_t _json_is_bin(int c) { JSON_U(c, 0);return _json_is_flag(c, jsoncf_bin); }
static inline  size_t _json_is_oct(int c) { JSON_U(c, 0);return _json_is_flag(c, jsoncf_oct); }
static inline  size_t _json_is_hex(int c) { JSON_U(c, 0);return _json_is_flag(c, jsoncf_hex); }
static inline  size_t _json_hex_leader(int c) { return c == 'x' || c == 'X'; }
static inline  size_t _json_bin_leader(int c) { return c == 'b' || c == 'B'; }

//提取名称
static inline LPCXSTR _json_collect_name(LPCXSTR _s, LPCXSTR _e, JSON_Value::Name & name)
{
	int nEndChar = 0;
	bool bquotation = *_s == '"' || *_s == '\'';
	if (bquotation)
	{
		nEndChar = *_s;
		_s++;

		name.start = _s;
		while (_s < _e && *_s != nEndChar)
		{
			if (*_s == '\\') _s++;
			_s++;
		}
		name.end = _s;

		if (_s < _e && *_s == nEndChar) _s++;
		else return nullptr;
	}
	else
	{
		name.start = _s;
		while (_s < _e && _json_is_name(*_s)) _s++;
		name.end = _s;
	}

	return _s;
}

//跳过起始的空白字符
//跳过注释(json5)
static inline LPCXSTR _json_shift_space(LPCXSTR _s, LPCXSTR _e)
{
#if JSON5_ENABLE_COMMENTS
	for (; _s < _e; )
	{
		if (_json_is_space(*_s))
		{
			++_s;
			continue;
		}
		else if (*_s != '/')
		{
			break;
		}

		++_s;
		if (_s >= _e)
			return _s;
		if (*_s == '/')
		{	// line comments
			while (_s < _e && *_s != 0 && *_s != '\r' && *_s != '\n') ++_s;
		}
		else if (*_s == '*')
		{	// block comments
			++_s;
			while (_s < _e && *_s != 0)
			{
				if (*_s == '*')
				{
					++_s;
					if (_s < _e && *_s == '/')
					{
						++_s;
						break;
					}
				}
				else
				{
					++_s;
				}
			}
		}
		else
		{
			--_s;
			break;
		}
	}

	return _s;
#else
	while (_s < _e && _json_is_space(*_s)) _s++;
	return _s;
#endif
}

//不区分大小写的比较字符串
static inline LPCXSTR _json_cmp_string(LPCXSTR _s, LPCXSTR _e, LPCXSTR psz)
{
	while ((_s < _e) && *psz && ((isupper(*(const XUCHAR*)_s) ? tolower(*_s) : *_s) == *psz))
	{
		++_s;
		++psz;
	}
	return *psz ? nullptr : _s;
}

JSON_Value * JSON_Parser::Parse(size_t nNunBatch, LPCXSTR psz, LPCXSTR * ppszEnd/* = nullptr*/)
{
	_json_init_char_flag();

	m_Alloctor.clear(nNunBatch);
	m_pRootValue = nullptr;

	if (psz == nullptr)
		return nullptr;

	LPCXSTR pszEnd;
	if (ppszEnd == nullptr)
		pszEnd = psz + _xcslen(psz);
	else
		pszEnd = *ppszEnd;

	m_pRootValue = parse_start(psz, pszEnd);

	if (ppszEnd != nullptr)
		*ppszEnd = psz;

	return m_pRootValue;
}

#define RET_NULL {psz = s; return nullptr;}
#define RET_ZERO {psz = s; return 0;}

JSON_Value * JSON_Parser::parse_pair(LPCXSTR& psz, LPCXSTR e)
{
	JSON_Value::Name name;
	LPCXSTR s = _json_collect_name(psz, e, name);
	if (s == nullptr) RET_NULL;
	size_t nlen = name.end - name.start;
	if (nlen > (std::numeric_limits<uint16_t>::max)()) RET_NULL;

	s = _json_shift_space(s, e);
	if (s >= e || (*s != ':' JSON5_IF_ENABLE(&& *s != '='))) RET_NULL;
	++s;

	JSON_Value * ret = parse_value(s, e);
	if (ret != nullptr)
	{
		ret->nlen = static_cast<uint16_t>(nlen);
		ret->name = name.start;
	}

	psz = s;
	return ret;
}

JSON_Value * JSON_Parser::parse_start(LPCXSTR& psz, LPCXSTR e)
{
	LPCXSTR s = _json_shift_space(psz, e);
	if (s >= e) RET_NULL;

	JSON_Value * ret = nullptr;
	if (*s == '{')
	{
		++s;
		ret = m_Alloctor.alloc();
		ret->type = JSONT_Object;
		ret = parse_object(ret, s, e);
	}
	else if(*s == '[')
	{
		++s;
		ret = m_Alloctor.alloc();
		ret->type = JSONT_Array;
		ret = parse_array(ret, s, e);
	}

	psz = s;
	return ret;
}

JSON_Value * JSON_Parser::parse_value(LPCXSTR& psz, LPCXSTR e)
{
	LPCXSTR s = _json_shift_space(psz, e);
	if (s >= e) RET_NULL;

	JSON_Value * ret = nullptr;
	switch (*s)
	{
	case '{':
	{
		++s;
		ret = parse_object(m_Alloctor.alloc(), s, e);
		if (ret != nullptr)
		{
			ret->type = JSONT_Object;
		}
	}
	break;
	case '[':
	{
		++s;
		ret = parse_array(m_Alloctor.alloc(), s, e);
		if (ret != nullptr)
		{
			ret->type = JSONT_Array;
		}
	}
	break;
	case '"':
	case '\'':
	{
		int nEndChar = *s;
		++s;
		ret = parse_string(s, e, nEndChar);
	}
	break;
	case 't':
	case 'T':
	{
		s = _json_cmp_string(s, e, X_T("true"));
		if (s != nullptr)
		{
			ret = m_Alloctor.alloc();
			ret->type = JSONT_Boolean;
			ret->i = 1;
		}
	}
	break;
	case 'f':
	case 'F':
	{
		s = _json_cmp_string(s, e, X_T("false"));
		if (s != nullptr)
		{
			ret = m_Alloctor.alloc();
			ret->type = JSONT_Boolean;
			ret->i = 0;
		}
	}
	break;
	case 'n':
	case 'N':
	{
		LPCXSTR st = _json_cmp_string(s, e, X_T("null"));
		if (st != nullptr)
		{
			ret = m_Alloctor.alloc();
			ret->type = JSONT_Nullptr;
		}
		else
		{	//json5 NaN
			st = _json_cmp_string(s, e, X_T("nan"));
			if (st != nullptr)
			{
				ret = m_Alloctor.alloc();
				ret->type = JSONT_Double;
				ret->f = NAN;
			}
		}
		s = st;
	}
	break;
	case 'i':
	case 'I':	//json5 Infinity
	{
		s = _json_cmp_string(s, e, X_T("infinity"));
		if (s != nullptr)
		{
			ret = m_Alloctor.alloc();
			ret->type = JSONT_Double;
			ret->f = INFINITY;
		}
	}
	break;
	case '+':
	case '-':
	{	//json5 -Infinity/-NaN
		LPCXSTR st = s + 1;
		if (st >= e) RET_NULL;
		if (*st == 'n' || *st == 'N')
		{
			st = _json_cmp_string(st, e, X_T("nan"));
			if (st != nullptr)
			{
				ret = m_Alloctor.alloc();
				ret->type = JSONT_Double;
				ret->f = (*s == '+') ? NAN : -NAN;
			}
			s = st;
			break;
		}
		else if (*st == 'i' || *st == 'I')
		{
			st = _json_cmp_string(st, e, X_T("infinity"));
			if (st != nullptr)
			{
				ret = m_Alloctor.alloc();
				ret->type = JSONT_Double;
				ret->f = (*s == '+') ? INFINITY : -INFINITY;
			}
			s = st;
			break;
		}
	}
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '.':	//json5
	{
		ret = parse_number(s, e);
	}
	break;
	}

	psz = s;
	return ret;
}

JSON_Value * JSON_Parser::parse_object(JSON_Value* parent, LPCXSTR& s, LPCXSTR e)
{
	parent->elements = nullptr;

	s = _json_shift_space(s, e);
	if (s >= e) return nullptr;
	if (*s == '}')
	{//empty object
		++s;
		return parent;
	}

	JSON_Value * ret;
	while ((ret = parse_pair(s, e)) != nullptr)
	{
		s = _json_shift_space(s, e);
		if (s >= e) return nullptr;

		ret->next = parent->elements;
		parent->elements = ret;

		if (*s == '}')
		{
			++s;
			return parent;
		}
		else if (*s == ',' JSON5_IF_ENABLE(|| *s == ';'))
		{
			++s;

			s = _json_shift_space(s, e);
			if (s >= e) return nullptr;
#if JSON_ENABLE_JSON5
			if (*s == '}')
			{
				++s;
				return parent;
			}
#endif
			continue;
		}
		else
		{
			return nullptr;
		}
	}
	return nullptr;
}

JSON_Value * JSON_Parser::parse_array(JSON_Value* parent, LPCXSTR& s, LPCXSTR e)
{
	parent->elements = nullptr;

	s = _json_shift_space(s, e);
	if (s >= e) return nullptr;
	if (*s == ']')
	{//empty array
		++s;
		return parent;
	}

	JSON_Value * pValue;
	while ((pValue = parse_value(s, e)) != nullptr)
	{
		s = _json_shift_space(s, e);
		if (s >= e) return nullptr;

		pValue->next = parent->elements;
		parent->elements = pValue;

		if (*s == ']')
		{
			++s;
			return parent;
		}
		else if (*s == ',' JSON5_IF_ENABLE(|| *s == ';'))
		{
			++s;

			s = _json_shift_space(s, e);
			if (s >= e) return nullptr;
#if JSON_ENABLE_JSON5
			if (*s == ']')
			{
				++s;
				return parent;
			}
#endif
			continue;
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

JSON_Value * JSON_Parser::parse_string(LPCXSTR& psz, LPCXSTR e, int nEndChar)
{
	LPCXSTR s = psz;
	for (;s < e && *s != nEndChar; ++s)
	{
		if (*s == '\\') ++s;
	}
	if (s < e && (s - psz) <= (std::numeric_limits<int32_t>::max)())
	{
		JSON_Value * ret = m_Alloctor.alloc();
		ret->type = JSONT_String;
		ret->slen = static_cast<int32_t>(s - psz);
		ret->str = psz;

		psz = s + 1;

		return ret;
	}
	return nullptr;
}

int64_t JSON_Parser::_parse_long(LPCXSTR& psz, LPCXSTR e, JSON_Type& eType)
{
	LPCXSTR s = psz, scanstart = psz;
	if (e == nullptr)
		e = s + _xcslen(s);

	int _Radix = 10;
	if (*s == '-' || *s == '+')
	{
		++s;
		if (s >= e) RET_ZERO;
	}
	else if (*s == '0' && s + 1 < e)
	{
		if (_json_hex_leader(s[1]))
		{
			s += 2;
			if (s >= e) RET_ZERO;
			scanstart = s;
			_Radix = 16;
		}
		else if (_json_bin_leader(s[1]))
		{
			s += 2;
			if (s >= e) RET_ZERO;
			scanstart = s;
			_Radix = 2;
		}
		else if (_json_is_oct(s[1]))
		{
			s += 1;
			scanstart = s;
			_Radix = 8;
		}
	}

	if (_Radix == 2)
		while (s < e && _json_is_bin(*s)) s++;
	else if (_Radix == 10)
		while (s < e && _json_is_digit(*s)) s++;
	else if (_Radix == 8)
		while (s < e && _json_is_oct(*s)) s++;
	else if (_Radix == 16)
		while (s < e && _json_is_hex(*s)) s++;

	int64_t lValue = _xcstoi64(scanstart, (LPXSTR *)&s, _Radix);
	eType = JSON_Type(JSONT_Long | ((_Radix - 1) << 4));
	psz = s;

	return lValue;
}

JSON_Value * JSON_Parser::parse_number(LPCXSTR& psz, LPCXSTR e)
{
	bool minus = false;
	LPCXSTR s = psz;
#if JSON_FAST_STRTOD
	LPCXSTR startscane = s;
#endif

	if (*s == '+')
	{//+开头，十进制或者浮点数
		++s;
		if (s >= e) RET_NULL;
	}
	else if (*s == '-')
	{//-开头，十进制或者浮点数
		++s;
		if (s >= e) RET_NULL;
		minus = true;
	}
	else if (*s == '0' && s + 1 < e)
	{//0开头，可能是0，十六进制，二进制，八进制
		if (_json_hex_leader(s[1]))
		{//0x，十六进制
			s += 2;
			if (s >= e) RET_NULL;

			uint64_t i64 = 0;
			while (s < e && _json_is_hex(*s) && i64 <= 0x0fffffffffffffffULL)
			{
				int32_t i = *s++;
				if (_json_is_digit(i))
					i = i - '0';
				else if (i >= 'a' && i <= 'f')
					i = i - 'a' + 10;
				else
					i = i - 'A' + 10;

				i64 = (i64 << 4) | i;
			}

			JSON_Value * ret = m_Alloctor.alloc();
			ret->type = JSONT_HexLong;
			ret->l = i64;

			psz = s;
			return ret;
		}
		else if (_json_bin_leader(s[1]))
		{//0b，二进制
			s += 2;
			if (s >= e) RET_NULL;

			uint64_t i64 = 0;
			while (s < e && _json_is_hex(*s) && i64 <= 0x7fffffffffffffffULL)
			{
				int32_t i = *s++;
				i = i - '0';

				i64 = (i64 << 1) | i;
			}

			JSON_Value * ret = m_Alloctor.alloc();
			ret->type = JSONT_BinaryLong;
			ret->l = i64;

			psz = s;
			return ret;
		}
		else if (_json_is_oct(s[1]))
		{//0[1-7]，八进制
			s += 1;

			uint64_t i64 = 0;
			while (s < e && _json_is_oct(*s) && i64 < 0x7fffffffffffffffULL)
			{
				int32_t i = *s++;
				i = i - '0';

				i64 = (i64 << 3) | i;
			}

			JSON_Value * ret = m_Alloctor.alloc();
			ret->type = JSONT_OctalLong;
			ret->l = i64;

			psz = s;
			return ret;
		}
		else if (s[1] != '.')
		{//0
			JSON_Value * ret = m_Alloctor.alloc();
			ret->type = JSONT_DecimalLong;
			ret->l = 0;

			psz = s + 1;
			return ret;
		}
	}

	//十进制整数或者浮点数
	uint64_t i64 = 0;
#if !JSON_FAST_STRTOD
	double d = 0.0;
	int significandDigit = 0;
#endif

	bool useDouble = false;
	while (s < e && _json_is_digit(*s))
	{
		if (i64 >= 0x0CCCCCCCCCCCCCCCULL)	// 2^63 = 9223372036854775808
		{
			if (i64 != 0x0CCCCCCCCCCCCCCCULL || *s >= '8')
			{
#if !JSON_FAST_STRTOD
				d = static_cast<double>(i64);
#endif
				useDouble = true;
				break;
			}
		}

		//i64 = i64 * 10 + (*s++ - '0');
		i64 = (i64 << 3) + (i64 << 1) + (*s++ - '0');
#if !JSON_FAST_STRTOD
		significandDigit++;
#endif
	}

#if !JSON_FAST_STRTOD
	if (useDouble)
	{
		while (s < e && _json_is_digit(*s))
		{
			if (d >= 1.7976931348623157e307)	// DBL_MAX / 10.0
				RET_NULL;

			d = d * 10.0 + (*s++ - '0');
		}
	}

	int expFrac = 0;
	if (s < e && *s == '.')
	{
		++s;
		if (s >= e) RET_NULL;

		//if (!_json_is_digit(*s)) RET_NULL;

		if (!useDouble)
		{
			while (s < e && _json_is_digit(*s))
			{
				if (i64 > 0x1FFFFFFFFFFFFFULL) //2^53 - 1 for fast path
					break;
				else
				{
					//i64 = i64 * 10 + (*s++ - '0');
					i64 = (i64 << 3) + (i64 << 1) + (*s++ - '0');
					--expFrac;
					if (i64 != 0)
						significandDigit++;
				}
			}

			d = static_cast<double>(i64);
			useDouble = true;
		}

		while (s < e && _json_is_digit(*s))
		{
			if (significandDigit < 17)
			{
				d = d * 10.0 + (*s - '0');
				--expFrac;
				if (d > 0.0)
					significandDigit++;
			}
			++s;
		}
	}

	int exp = 0;
	if (s < e && (*s | 32) == 'e')
	{
		++s;

		if (!useDouble)
		{
			d = static_cast<double>(i64);
			useDouble = true;
		}

		bool expMinus = false;
		if (s < e && *s == '+')
		{
			++s;
		}
		else if (s < e && *s == '-')
		{
			++s;
			expMinus = true;
		}

		if (s < e && _json_is_digit(*s))
		{
			exp = *s++ - '0';
			if (expMinus)
			{
				while (s < e && _json_is_digit(*s))
				{
					//exp = exp * 10 + (*s++ - '0');
					exp = (exp << 3) + (exp << 1) + (*s++ - '0');
					if (exp >= 214748364)
					{
						while (s < e && _json_is_digit(*s))
							++s;
					}
				}
			}
			else
			{
				int maxExp = 308 - expFrac;
				while (s < e && _json_is_digit(*s))
				{
					//exp = exp * 10 + (*s++ - '0');
					exp = (exp << 3) + (exp << 1) + (*s++ - '0');
					if (exp > maxExp)
						RET_NULL;
				}
			}
		}
		else
		{
			RET_NULL;
		}

		if (expMinus)
			exp = -exp;
	}

	JSON_Value * ret = nullptr;
	if (useDouble)
	{
		int p = exp + expFrac;
		d = StrtodNormalPrecision(d, p);

		ret = m_Alloctor.alloc();
		ret->type = JSONT_Double;
		ret->f = minus ? -d : d;
	}
	else
	{
		ret = m_Alloctor.alloc();
		ret->type = JSONT_DecimalLong;
		if (minus)
			ret->l = static_cast<int64_t>(~i64 + 1);
		else
			ret->l = static_cast<int64_t>(i64);
	}
	
#else

	if (!useDouble && s < e)
	{
		if (*s == '.' || (*s | 32) == 'e')
		{
			useDouble = true;
		}
	}

	JSON_Value * ret = nullptr;
	if (useDouble)
	{
		ret = m_Alloctor.alloc();
		ret->type = JSONT_Double;
		ret->f = _tcstod_fast(startscane, (XCHAR **)&s);
	}
	else
	{
		ret = m_Alloctor.alloc();
		ret->type = JSONT_DecimalLong;
		if (minus)
			ret->l = static_cast<int64_t>(~i64 + 1);
		else
			ret->l = static_cast<int64_t>(i64);
	}
#endif

	psz = s;
	return ret;
}

VFX_API size_t JSON_ElementsCount(const JSON_Value& jv)
{
	if (jv.type == JSONT_Object || jv.type == JSONT_Array)
	{
		size_t nCount = 0;
		for (JSON_Value* e = jv.elements; e != nullptr; e = e->next) ++nCount;
		return nCount;
	}
	return 0;
}

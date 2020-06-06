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
	: m_pRootValue(nullptr)
	, m_pError(nullptr)
{
}

JSON_Parser::~JSON_Parser()
{

}

void JSON_Parser::set_error(LPCXSTR e) noexcept
{
	m_pError = e;
}

static inline size_t _json_is_flag(int c, size_t f) noexcept { return __json_char_flag[(unsigned char)c] & f; }
static inline  size_t _json_is_space(int c) noexcept { JSON_U(c, 0);return _json_is_flag(c, jsoncf_space); }
static inline  size_t _json_is_name(int c) noexcept { JSON_U(c, 1);return _json_is_flag(c, jsoncf_name); }
static inline  size_t _json_is_digit(int c) noexcept { JSON_U(c, 0);return _json_is_flag(c, jsoncf_digit); }
static inline  size_t _json_is_bin(int c) noexcept { JSON_U(c, 0);return _json_is_flag(c, jsoncf_bin); }
static inline  size_t _json_is_oct(int c) noexcept { JSON_U(c, 0);return _json_is_flag(c, jsoncf_oct); }
static inline  size_t _json_is_hex(int c) noexcept { JSON_U(c, 0);return _json_is_flag(c, jsoncf_hex); }
static inline  size_t _json_hex_leader(int c) noexcept { return c == 'x' || c == 'X'; }
static inline  size_t _json_bin_leader(int c) noexcept { return c == 'b' || c == 'B'; }

//提取名称
LPCXSTR JSON_Parser::_json_collect_name(LPCXSTR _s, LPCXSTR _e, JSON_Value::Name & name) noexcept
{
	int nEndChar = 0;
	bool bquotation = *_s == '"' JSON5_IF_ENABLE(|| *_s == '\'');
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

		if (_s < _e && *_s == nEndChar) 
			_s++;
		else
		{
			set_error(X_T("keys must be quoted."));
			return nullptr;
		}
	}
	else
	{
#if JSON_ENABLE_JSON5
		name.start = _s;
		while (_s < _e && _json_is_name(*_s)) _s++;
		name.end = _s;
#else
		set_error(X_T("keys must be quoted."));
		return nullptr;
#endif
	}

	return _s;
}

//跳过起始的空白字符
//跳过注释(json5)
static inline LPCXSTR _json_shift_space(LPCXSTR _s, LPCXSTR _e) noexcept
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
static inline LPCXSTR _json_cmp_string(LPCXSTR _s, LPCXSTR _e, LPCXSTR psz) noexcept
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

#if !JSON_ENABLE_JSON5
	if (m_pRootValue != nullptr && psz != pszEnd)
	{
		psz = _json_shift_space(psz, pszEnd);
		if (psz != pszEnd)
		{
			set_error(X_T("more characters after the close."));
		}
	}
#endif

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
	if (s == nullptr)
		return nullptr;
	size_t nlen = name.end - name.start;
	if (nlen > (std::numeric_limits<uint16_t>::max)())
	{
		set_error(X_T("key length exceeds 65535."));
		RET_NULL;
	}

	s = _json_shift_space(s, e);
	if (s >= e || (*s != ':' JSON5_IF_ENABLE(&& *s != '=')))
	{
		set_error(X_T("missing colon."));
		RET_NULL;
	}
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
		ret->type = JSON_Type::Object;
		ret = parse_object(ret, s, e);
	}
	else if(*s == '[')
	{
		++s;
		ret = m_Alloctor.alloc();
		ret->type = JSON_Type::Array;
		ret = parse_array(ret, s, e);
	}
	else
	{
		set_error(X_T("A JSON payload should be an object or array."));
	}

	psz = s;
	return ret;
}

JSON_Value * JSON_Parser::parse_value(LPCXSTR& psz, LPCXSTR e)
{
	LPCXSTR s = _json_shift_space(psz, e);
	if (s >= e) RET_NULL;

	LPCXSTR st;
	JSON_Value * ret = nullptr;
	switch (*s)
	{
	case '{':
		{
			++s;
			ret = parse_object(m_Alloctor.alloc(), s, e);
			if (ret != nullptr)
			{
				ret->type = JSON_Type::Object;
			}
		}
		break;
	case '[':
		{
			++s;
			ret = parse_array(m_Alloctor.alloc(), s, e);
			if (ret != nullptr)
			{
				ret->type = JSON_Type::Array;
			}
		}
		break;
	case '"':
		{
			int nEndChar = *s;
			++s;
			ret = parse_string(s, e, nEndChar);
		}
		break;
	case '\'':
		{
#if JSON_ENABLE_JSON5
			int nEndChar = *s;
			++s;
			ret = parse_string(s, e, nEndChar);
#else
			set_error(X_T("illegal single quote."));
#endif
		}
		break;
	case 't':
	case 'T':
		{
			st = _json_cmp_string(s, e, X_T("true"));
			if (st != nullptr)
			{
				ret = m_Alloctor.alloc();
				ret->type = JSON_Type::Boolean;
				ret->i = 1;
				s = st;
			}
			else
			{
#if JSON_ENABLE_JSON5
				goto __label_as_string;
#else
				set_error(X_T("bad value."));
#endif
			}
		}
		break;
	case 'f':
	case 'F':
		{
			st = _json_cmp_string(s, e, X_T("false"));
			if (st != nullptr)
			{
				ret = m_Alloctor.alloc();
				ret->type = JSON_Type::Boolean;
				ret->i = 0;
				s = st;
			}
			else
			{
#if JSON_ENABLE_JSON5
				goto __label_as_string;
#else
				set_error(X_T("bad value."));
#endif
			}
		}
		break;
	case 'n':
	case 'N':
		{
			st = _json_cmp_string(s, e, X_T("null"));
			if (st != nullptr)
			{
				ret = m_Alloctor.alloc();
				ret->type = JSON_Type::Nullptr;
				s = st;
			}
			else
			{	//json5 NaN
				st = _json_cmp_string(s, e, X_T("nan"));
				if (st != nullptr)
				{
					ret = m_Alloctor.alloc();
					ret->type = JSON_Type::Double;
					ret->f = NAN;
					s = st;
				}
				else
				{
#if JSON_ENABLE_JSON5
					goto __label_as_string;
#else
					set_error(X_T("bad value."));
#endif
				}
			}
		}
		break;
	case 'i':
	case 'I':	//json5 Infinity
		{
			st = _json_cmp_string(s, e, X_T("infinity"));
			if (st != nullptr)
			{
				ret = m_Alloctor.alloc();
				ret->type = JSON_Type::Double;
				ret->f = INFINITY;
				s = st;
			}
			else
			{
#if JSON_ENABLE_JSON5
				goto __label_as_string;
#else
				set_error(X_T("bad value."));
#endif
			}
		}
		break;
	case '+':
	case '-':
		{	//json5 -Infinity/-NaN
			st = s + 1;
			if (st >= e) RET_NULL;
			if (*st == 'n' || *st == 'N')
			{
				st = _json_cmp_string(st, e, X_T("nan"));
				if (st != nullptr)
				{
					ret = m_Alloctor.alloc();
					ret->type = JSON_Type::Double;
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
					ret->type = JSON_Type::Double;
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
			ret = m_Alloctor.alloc();
			ret = parse_number(ret, s, e);
		}
		break;
	case ',':
		{
			set_error(X_T("extra comma."));
		}
		break;
	default:
		{
#if JSON_ENABLE_JSON5
		__label_as_string:
			if (_json_is_name(*s))
			{
				JSON_Value::Name name;
				s = _json_collect_name(s, e, name);
				if (s != nullptr)
				{
					if (name.end - name.start > (std::numeric_limits<int32_t>::max)())
					{
						set_error(X_T("string length exceeds 2147483648."));
					}
					else
					{

						ret = m_Alloctor.alloc();
						ret->type = JSON_Type::String;
						ret->slen = static_cast<uint32_t>(name.end - name.start);
						ret->str = name.start;
					}
				}
			}
#else
			set_error(X_T("illegal expression."));
#endif
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
	if (s >= e)
	{
		set_error(X_T("unclosed object."));
		return nullptr;
	}
	if (*s == '}')
	{//empty object
		++s;
		return parent;
	}

	JSON_Value * ret;
	while ((ret = parse_pair(s, e)) != nullptr)
	{
		s = _json_shift_space(s, e);
		if (s >= e)
		{
			set_error(X_T("unclosed object."));
			return nullptr;
		}

		ret->prev = parent->elements;
		parent->elements = ret;

		if (*s == '}')
		{
			++s;
			return parent;
		}
		else if (*s == ',' JSON5_IF_ENABLE(|| *s == ';'))
		{
#if JSON_ENABLE_JSON5
__loop_json5_object_comma :
#endif
			++s;

			s = _json_shift_space(s, e);
			if (s >= e)
			{
				set_error(X_T("unclosed object."));
				return nullptr;
			}
#if JSON_ENABLE_JSON5
			if (*s == ',' || *s == ';')
				goto __loop_json5_object_comma;
#endif
			if (*s == '}')
			{
#if JSON_ENABLE_JSON5
				++s;
				return parent;
#else
				set_error(X_T("extra comma."));
				return nullptr;
#endif
			}
			continue;
		}
		else
		{
			set_error(X_T("missing comma."));
			return nullptr;
		}
	}

	if (parent->elements == nullptr)
	{
		if(m_pError == nullptr)
			set_error(X_T("missing value."));
	}
	return nullptr;
}

JSON_Value * JSON_Parser::parse_array(JSON_Value* parent, LPCXSTR& s, LPCXSTR e)
{
	parent->elements = nullptr;

	s = _json_shift_space(s, e);
	if (s >= e)
	{
		set_error(X_T("unclosed array."));
		return nullptr;
	}
	if (*s == ']')
	{//empty array
		++s;
		return parent;
	}

	JSON_Value * ret;
	while ((ret = parse_value(s, e)) != nullptr)
	{
		s = _json_shift_space(s, e);
		if (s >= e)
		{
			set_error(X_T("unclosed array."));
			return nullptr;
		}

		ret->prev = parent->elements;
		parent->elements = ret;

		if (*s == ']')
		{
			++s;
			return parent;
		}
		else if (*s == ',' JSON5_IF_ENABLE(|| *s == ';'))
		{
#if JSON_ENABLE_JSON5
__loop_json5_array_comma :
#endif
			++s;

			s = _json_shift_space(s, e);
			if (s >= e)
			{
				set_error(X_T("unclosed array."));
				return nullptr;
			}
#if JSON_ENABLE_JSON5
			if (*s == ',' || *s == ';')
				goto __loop_json5_array_comma;
#endif
			if (*s == ']')
			{
#if JSON_ENABLE_JSON5
				++s;
				return parent;
#else
				set_error(X_T("extra comma."));
				return nullptr;
#endif
			}
			continue;
		}
		else
		{
			set_error(X_T("missing comma."));
			return nullptr;
		}
	}

	if (parent->elements == nullptr)
	{
		if (m_pError == nullptr)
			set_error(X_T("missing value."));
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
		ret->type = JSON_Type::String;
		ret->slen = static_cast<int32_t>(s - psz);
		ret->str = psz;

		psz = s + 1;

		return ret;
	}

	if (s >= e)
	{
		set_error(X_T("unexpected end."));
	}
	else
	{
		set_error(X_T("string length exceeds 2147483648."));
	}

	return nullptr;
}

int64_t JSON_Parser::_parse_long(LPCXSTR& psz, LPCXSTR e, JSON_Type& eType) noexcept
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

	int64_t lValue = _xcstoll(scanstart, (LPXSTR *)&s, _Radix);
	eType = JSON_Type::Long | (JSON_Type)((_Radix - 1) << 4);
	psz = s;

	return lValue;
}

JSON_Value * JSON_Parser::parse_number(JSON_Value* ret, LPCXSTR& psz, LPCXSTR e) noexcept
{
	bool minus = false;
	LPCXSTR s = psz;

	if (*s == '+')
	{//+开头，十进制或者浮点数
		++s;
		if (s >= e)
		{
			set_error(X_T("unexpected end."));
			RET_NULL;
		}
	}
	else if (*s == '-')
	{//-开头，十进制或者浮点数
		++s;
		if (s >= e)
		{
			set_error(X_T("unexpected end."));
			RET_NULL;
		}
		minus = true;
	}
	else if (*s == '0' && s + 1 < e)
	{//0开头，可能是0，十六进制，二进制，八进制
		if (s[1] != '.' && s[1] != 'e' && s[1] != 'E')
		{
#if JSON_ENABLE_JSON5
			if (_json_hex_leader(s[1]))
			{//0x，十六进制
#if JSON_ENABLE_JSON5
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

				ret->type = JSON_Type::HexLong;
				ret->l = i64;

				psz = s;
				return ret;
#else
				set_error(X_T("numbers cannot be hex."));
				return nullptr;
#endif
			}
			if (_json_bin_leader(s[1]))
			{//0b，二进制
#if JSON_ENABLE_JSON5
				s += 2;
				if (s >= e) RET_NULL;

				uint64_t i64 = 0;
				while (s < e && _json_is_hex(*s) && i64 <= 0x7fffffffffffffffULL)
				{
					int32_t i = *s++;
					i = i - '0';

					i64 = (i64 << 1) | i;
				}

				ret->type = JSON_Type::BinaryLong;
				ret->l = i64;

				psz = s;
				return ret;
#else
				set_error(X_T("numbers cannot be binary."));
				return nullptr;
#endif
			}
			if (_json_is_oct(s[1]))
			{//0[1-7]，八进制
#if JSON_ENABLE_JSON5
				s += 1;

				uint64_t i64 = 0;
				while (s < e && _json_is_oct(*s) && i64 < 0x7fffffffffffffffULL)
				{
					int32_t i = *s++;
					i = i - '0';

					i64 = (i64 << 3) | i;
				}

				ret->type = JSON_Type::OctalLong;
				ret->l = i64;

				psz = s;
				return ret;
#else
				set_error(X_T("numbers cannot be octal."));
				return nullptr;
#endif
			}
#endif
			if(_json_is_digit(s[1]))
			{//089
				;
#if !JSON_ENABLE_JSON5
				set_error(X_T("numbers cannot have leading zeroes."));
				return nullptr;
#endif
			}
		}
	}

	//十进制整数或者浮点数
	uint64_t i64 = 0;
	double d = 0.0;
	int significandDigit = 0;

	bool useDouble = false;
	while (s < e && _json_is_digit(*s))
	{
		if (i64 >= 0x0CCCCCCCCCCCCCCCULL)	// 2^63 = 9223372036854775808
		{
			if (i64 != 0x0CCCCCCCCCCCCCCCULL || *s >= '8')
			{
				d = static_cast<double>(i64);
				useDouble = true;
				break;
			}
		}

		int dgt = *s++ - '0';
		i64 = i64 * 10 + dgt;
		significandDigit++;
	}

	if (useDouble)
	{
		while (s < e && _json_is_digit(*s))
		{
			if (d >= 1.7976931348623157e307)	// DBL_MAX / 10.0
			{
				set_error(X_T("numbers out of range."));
				return nullptr;
			}

			int dgt = *s++ - '0';
			d = d * 10.0 + dgt;
		}
	}

	int expFrac = 0;
	if (s < e && *s == '.')
	{
		++s;
		if (s >= e)
		{
			set_error(X_T("unexpected end."));
			RET_NULL;
		}

		//if (!_json_is_digit(*s)) RET_NULL;

		if (!useDouble)
		{
			int dgt;
			while (s < e && _json_is_digit(dgt = *s))
			{
				if (i64 <= 0x1FFFFFFFFFFFFFULL) //2^53 - 1 for fast path
				{
					++s;
					--expFrac;
					i64 = i64 * 10 + dgt - '0';
					if (i64 != 0)
						significandDigit++;
				}
				else
				{
					break;
				}
			}

			d = static_cast<double>(i64);
			useDouble = true;
		}

		int dgt;
		while (s < e && _json_is_digit(dgt = *s))
		{
			if (significandDigit < 17)
			{
				--expFrac;
				dgt -= '0';
				d = d * 10.0 + dgt;
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
					int dgt = *s++ - '0';
					exp = exp * 10 + dgt;
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
					int dgt = *s++ - '0';
					exp = exp * 10 + dgt;
					if (exp > maxExp)
					{
						set_error(X_T("numbers out of range."));
						return nullptr;
					}
				}
			}
		}
		else
		{
			set_error(X_T("illegal number."));
			return nullptr;
		}

		if (expMinus)
			exp = -exp;
	}

	if (useDouble)
	{
		int p = exp + expFrac;
		d = StrtodNormalPrecision(d, p);

		ret->type = JSON_Type::Double;
		ret->f = minus ? -d : d;
	}
	else
	{
		ret->type = JSON_Type::DecimalLong;
		if (minus)
			ret->l = static_cast<int64_t>(~i64 + 1);
		else
			ret->l = static_cast<int64_t>(i64);
	}

	psz = s;
	return ret;
}

size_t JSON_ElementsCount(const JSON_Value* jv)
{
	if (jv->type == JSON_Type::Object || jv->type == JSON_Type::Array)
	{
		size_t nCount = 0;
		for (JSON_Value* e = jv->elements; e != nullptr; e = e->prev) ++nCount;
		return nCount;
	}
	return 0;
}

LPXSTR JSON_LoadString(LPXSTR pszStart, LPCXSTR s, LPCXSTR e)
{
	LPXSTR psz = pszStart;
	for (; s < e; ++s)
	{
		if (*s == '\\')
		{
			++s;
			if (s < e)
			{
				switch (*s)
				{
				default: *psz++ = *s;  break;
				case 'b':*psz++ = '\b'; break;
				case 'f':*psz++ = '\f'; break;
				case 't':*psz++ = '\t'; break;
				case 'n':*psz++ = '\n'; break;
				case '\n':*psz++ = '\n'; break;
				case 'r':*psz++ = '\r'; break;
				case '\r':*psz++ = '\r'; break;
				case 'u':
					if (s + 4 < e)
					{
						++s;
						uint32_t wc = 0;
						for (size_t k = 0; k < 4; ++k)
						{
							uint32_t c = *s++;

							if (c >= '0' && c <= '9')
								c = c - '0';
							else if (c >= 'a' && c <= 'f')
								c = c - 'a' + 10;
							else if (c >= 'A' && c <= 'F')
								c = c - 'A' + 10;
							else
							{
								wc = 0;
								break;
							}

							wc = (wc << 4) | c;
						}
						--s;

						if (wc != 0)
						{
#pragma warning(disable : 4244)
							if (sizeof(*psz) == sizeof(char))
							{
								char buffer[8];
								//int nCvt = sizeof(buffer);
								//wctomb_s(&nCvt, buffer, sizeof(buffer), wc);
								int nCvt = wctomb(buffer, wc);
								for (int x = 0; x < nCvt; ++x)
									*psz++ = buffer[x];
							}
							else if (sizeof(*psz) == sizeof(char16_t))
							{
								*psz++ = wc;
								if (wc > 0xffffu)
									*psz++ = wc >> 16;
							}
							else if (sizeof(*psz) == sizeof(char32_t))
							{
								*psz++ = (char32_t)wc + 0x10000;
							}
							else
							{
								assert(false);
							}
#pragma warning(default : 4244)
						}
					}
					break;	//\u5efa\u7b51 TODO 处理Unicode的读取
				}
			}
			else
			{
				*psz++ = '\\';
			}
		}
		else
		{
			*psz++ = *s;
		}
	}

	return psz;
}

std::basic_string<XCHAR> JSON_GetName(const JSON_Value* jv)
{
	std::basic_string<XCHAR> name;

	name.resize(jv->nlen);
	auto e = JSON_LoadString(const_cast<XCHAR*>(name.data()), jv->name, jv->name + jv->nlen);
	assert(e - name.data() <= jv->nlen);
	*e = 0;
	name.resize(e - name.data());

	return name;
}

std::basic_string<XCHAR> JSON_GetString(const JSON_Value* jv)
{
	std::basic_string<XCHAR> name;

	if (JSON_GetType(jv) == JSON_Type::String)
	{
		name.resize(jv->slen);
		auto e = JSON_LoadString(const_cast<XCHAR*>(name.data()), jv->str, jv->str + jv->slen);
		assert(e - name.data() <= jv->slen);
		*e = 0;
		name.resize(e - name.data());
	}

	return name;
}

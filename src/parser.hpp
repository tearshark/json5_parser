﻿//2018-01-09  支持 JSON5 规范

inline void JSON_Parser::set_error(LPCXSTR e) noexcept
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
LPCXSTR JSON_Parser::_json_collect_name(LPCXSTR _s, LPCXSTR _e, JSON_String& name) noexcept
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

bool JSON_Parser::Parse(JSON_Walker* walker, LPCXSTR psz, LPCXSTR * ppszEnd/* = nullptr*/)
{
	assert(walker != nullptr);

	m_pWalker = walker;

	if (psz == nullptr)
		return false;

	LPCXSTR pszEnd;
	if (ppszEnd == nullptr)
		pszEnd = psz + _xcslen(psz);
	else
		pszEnd = *ppszEnd;

	bool ret = parse_start(psz, pszEnd);

#if !JSON_ENABLE_JSON5
	if (ret != false && psz != pszEnd)
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

	if (!ret)
		m_pWalker->ErrorStop(m_pError, psz);

	return ret;
}

#define RET_NULL {psz = s; return false;}
#define RET_ZERO {psz = s; return 0;}

bool JSON_Parser::parse_pair(LPCXSTR& psz, LPCXSTR e)
{
	JSON_String name;
	LPCXSTR s = _json_collect_name(psz, e, name);
	if (s == nullptr)
		return false;

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

	m_pWalker->PushName(name);

	bool ret = parse_value(s, e);

	psz = s;
	return ret;
}

bool JSON_Parser::parse_start(LPCXSTR& psz, LPCXSTR e)
{
	LPCXSTR s = _json_shift_space(psz, e);
	if (s >= e)
	{
		set_error(X_T("unexpected end."));
		RET_NULL;
	}

	bool ret = false;
	if (*s == '{')
	{
		++s;
		void* parent = m_pWalker->PushObject(true);
		ret = parse_object(parent, s, e);
	}
	else if(*s == '[')
	{
		++s;
		void* parent = m_pWalker->PushArray(true);
		ret = parse_array(parent, s, e);
	}
	else
	{
		set_error(X_T("A JSON payload should be an object or array."));
	}

	psz = s;
	return ret;
}

bool JSON_Parser::parse_value(LPCXSTR& psz, LPCXSTR e)
{
	LPCXSTR s = _json_shift_space(psz, e);
	if (s >= e)
	{
		set_error(X_T("unexpected end."));
		RET_NULL;
	}

	LPCXSTR st;
	bool ret = false;
	switch (*s)
	{
	case '{':
		{
			++s;
			void* parent = m_pWalker->PushObject(false);
			ret = parse_object(parent, s, e);
		}
		break;
	case '[':
		{
			++s;
			void* parent = m_pWalker->PushArray(false);
			ret = parse_array(parent, s, e);
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
				s = st;
				ret = true;
				m_pWalker->PushBoolean(true);
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
				s = st;
				ret = true;
				m_pWalker->PushBoolean(false);
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
				s = st;
				ret = true;
				m_pWalker->PushNull();
			}
			else
			{	//json5 NaN
				st = _json_cmp_string(s, e, X_T("nan"));
				if (st != nullptr)
				{
					s = st;
					ret = true;
					m_pWalker->PushDouble(NAN);
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
				s = st;
				ret = true;
				m_pWalker->PushDouble(INFINITY);
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
			if (st >= e)
			{
				set_error(X_T("unexpected end."));
				RET_NULL;
			}
			if (*st == 'n' || *st == 'N')
			{
				st = _json_cmp_string(st, e, X_T("nan"));
				if (st != nullptr)
				{
					ret = true;
					m_pWalker->PushDouble((*s == '+') ? NAN : -NAN);
				}
				s = st;
				break;
			}
			else if (*st == 'i' || *st == 'I')
			{
				st = _json_cmp_string(st, e, X_T("infinity"));
				if (st != nullptr)
				{
					ret = true;
					m_pWalker->PushDouble((*s == '+') ? INFINITY : -INFINITY);
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
						ret = true;
						m_pWalker->PushString(name.start, name.end - name.start);
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

bool JSON_Parser::parse_object(void* const parent, LPCXSTR& s, LPCXSTR e)
{
	//parent->type = JSON_Type::Object;
	//parent->elements = nullptr;

	s = _json_shift_space(s, e);
	if (s >= e)
	{
		set_error(X_T("unclosed object."));
		return false;
	}
	if (*s == '}')
	{//empty object
		++s;
		m_pWalker->PopObject(parent);
		return true;
	}

	bool ret;
	while ((ret = parse_pair(s, e)) != false)
	{
		s = _json_shift_space(s, e);
		if (s >= e)
		{
			set_error(X_T("unclosed object."));
			return false;
		}

		//ret->prev = parent->elements;
		//parent->elements = ret;

		if (*s == '}')
		{
			++s;
			m_pWalker->PopObject(parent);
			return true;
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
				return false;
			}
#if JSON_ENABLE_JSON5
			if (*s == ',' || *s == ';')
				goto __loop_json5_object_comma;
#endif
			if (*s == '}')
			{
#if JSON_ENABLE_JSON5
				++s;
				m_pWalker->PopObject(parent);
				return true;
#else
				set_error(X_T("extra comma."));
				return false;
#endif
			}
			continue;
		}
		else
		{
			set_error(X_T("missing comma."));
			return false;
		}
	}

/*
	if (parent->elements == nullptr)
	{
		if(m_pError == nullptr)
			set_error(X_T("missing value."));
	}
*/
	return false;
}

bool JSON_Parser::parse_array(void* const parent, LPCXSTR& s, LPCXSTR e)
{
	//parent->type = JSON_Type::Object;
	//parent->elements = nullptr;

	s = _json_shift_space(s, e);
	if (s >= e)
	{
		set_error(X_T("unclosed array."));
		return false;
	}
	if (*s == ']')
	{//empty array
		++s;
		m_pWalker->PopArray(parent);
		return true;
	}

	bool ret;
	while ((ret = parse_value(s, e)) != false)
	{
		s = _json_shift_space(s, e);
		if (s >= e)
		{
			set_error(X_T("unclosed array."));
			return false;
		}

		//ret->prev = parent->elements;
		//parent->elements = ret;

		if (*s == ']')
		{
			++s;
			m_pWalker->PopArray(parent);
			return true;
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
				return false;
			}
#if JSON_ENABLE_JSON5
			if (*s == ',' || *s == ';')
				goto __loop_json5_array_comma;
#endif
			if (*s == ']')
			{
#if JSON_ENABLE_JSON5
				++s;
				m_pWalker->PopArray(parent);
				return true;
#else
				set_error(X_T("extra comma."));
				return false;
#endif
			}
			continue;
		}
		else
		{
			set_error(X_T("missing comma."));
			return false;
		}
	}

/*
	if (parent->elements == nullptr)
	{
		if (m_pError == nullptr)
			set_error(X_T("missing value."));
	}
*/
	return false;
}

bool JSON_Parser::parse_string(LPCXSTR& psz, LPCXSTR e, int nEndChar)
{
	LPCXSTR s = psz;
	for (;s < e && *s != nEndChar; ++s)
	{
		if (*s == '\\') ++s;
	}

	if (s < e && (s - psz) <= (std::numeric_limits<int32_t>::max)())
	{
		m_pWalker->PushString(JSON_String{ psz, s });
		psz = s + 1;

		return true;
	}
	
	if (s >= e)
	{
		set_error(X_T("unexpected end."));
	}
	else
	{
		set_error(X_T("string length exceeds 2147483648."));
	}
	return false;
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

bool JSON_Parser::parse_number(LPCXSTR& psz, LPCXSTR e) noexcept
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
				if (s >= e)
				{
					set_error(X_T("unexpected end."));
					RET_NULL;
				}

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
				if (s >= e)
				{
					set_error(X_T("unexpected end."));
					RET_NULL;
				}

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

		m_pWalker->PushDouble(minus ? -d : d);
	}
	else
	{
		int64_t l = minus ? static_cast<int64_t>(~i64 + 1) : static_cast<int64_t>(i64);
		m_pWalker->PushLong(JSON_Type::DecimalLong, l);
	}

	psz = s;
	return true;
}
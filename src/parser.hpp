﻿//2018-01-09  支持 JSON5 规范

inline void js_parser::set_error(LPCXSTR e) noexcept
{
    m_pError = e;
}

static inline size_t _json_is_flag(int c, size_t f) noexcept { return __json_char_flag[(unsigned char)c] & f; }
static inline  size_t _json_is_space(int c) noexcept { JSON_U(c, 0); return _json_is_flag(c, jsoncf_space); }
static inline  size_t _json_is_name(int c) noexcept { JSON_U(c, 1); return _json_is_flag(c, jsoncf_name); }
static inline  size_t _json_is_digit(int c) noexcept { JSON_U(c, 0); return _json_is_flag(c, jsoncf_digit); }
static inline  size_t _json_is_bin(int c) noexcept { JSON_U(c, 0); return _json_is_flag(c, jsoncf_bin); }
static inline  size_t _json_is_oct(int c) noexcept { JSON_U(c, 0); return _json_is_flag(c, jsoncf_oct); }
static inline  size_t _json_is_hex(int c) noexcept { JSON_U(c, 0); return _json_is_flag(c, jsoncf_hex); }
static inline  size_t _json_hex_leader(int c) noexcept { return c == 'x' || c == 'X'; }
static inline  size_t _json_bin_leader(int c) noexcept { return c == 'b' || c == 'B'; }

//提取名称
LPCXSTR js_parser::_json_collect_name(LPCXSTR _s, LPCXSTR _e, std::basic_string_view<XCHAR>& name) noexcept
{
    int nEndChar = 0;
    bool bquotation = *_s == '"' JSON5_IF_ENABLE(|| *_s == '\'');
    if (bquotation)
    {
        nEndChar = *_s;
        _s++;

        LPCXSTR const _start = _s;
        while (_s < _e && *_s != nEndChar)
        {
            if (*_s == '\\') _s++;
            _s++;
        }
        name = std::basic_string_view<XCHAR>{ _start, static_cast<size_t>(_s - _start) };

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
        LPCXSTR const _start = _s;
        while (_s < _e && _json_is_name(*_s)) _s++;
        name = std::basic_string_view<XCHAR>{ _start, static_cast<size_t>(_s - _start) };
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
#endif	//JSON5_ENABLE_COMMENTS
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

bool js_parser::parse(js_sax_handler* handler, LPCXSTR psz, LPCXSTR* ppszEnd/* = nullptr*/)
{
    assert(handler != nullptr);

    m_pWalker = handler;

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
        m_pWalker->parse_error(m_pError, psz);

    return ret;
}

#define RET_NULL {psz = s; return false;}
#define RET_ZERO {psz = s; return 0;}

bool js_parser::parse_pair(LPCXSTR& psz, LPCXSTR e)
{
    std::basic_string_view<XCHAR> name;
    LPCXSTR s = _json_collect_name(psz, e, name);
    if (s == nullptr)
        return false;

    size_t nlen = name.size();
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

    m_pWalker->key(name);

    bool ret = parse_value(s, e);

    psz = s;
    return ret;
}

bool js_parser::parse_start(LPCXSTR& psz, LPCXSTR e)
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
        void* parent = m_pWalker->start_object(true);
        ret = parse_object(parent, s, e);
    }
    else if (*s == '[')
    {
        ++s;
        void* parent = m_pWalker->start_array(true);
        ret = parse_array(parent, s, e);
    }
    else
    {
        set_error(X_T("A JSON payload should be an object or array."));
    }

    psz = s;
    return ret;
}

bool js_parser::parse_value(LPCXSTR& psz, LPCXSTR e)
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
        void* parent = m_pWalker->start_object(false);
        ret = parse_object(parent, s, e);
    }
    break;
    case '[':
    {
        ++s;
        void* parent = m_pWalker->start_array(false);
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
            m_pWalker->boolean(true);
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
            m_pWalker->boolean(false);
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
            m_pWalker->null();
        }
        else
        {	//json5 NaN
            st = _json_cmp_string(s, e, X_T("nan"));
            if (st != nullptr)
            {
                s = st;
                ret = true;
                m_pWalker->number_float(std::numeric_limits<double>::quiet_NaN());
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
            m_pWalker->number_float(INFINITY);
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
                m_pWalker->number_float((*s == '+') ? NAN : -NAN);
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
                m_pWalker->number_float((*s == '+') ? INFINITY : -INFINITY);
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
        __label_as_string :
        if (_json_is_name(*s))
        {
            std::basic_string_view<XCHAR> name;
            s = _json_collect_name(s, e, name);
            if (s != nullptr)
            {
                if (name.size() > (std::numeric_limits<int32_t>::max)())
                {
                    set_error(X_T("string length exceeds 2147483648."));
                }
                else
                {
                    ret = true;
                    m_pWalker->string(name);
                }
            }
            break;
        }
#endif
        set_error(X_T("illegal expression."));
    }
    break;
    }

    psz = s;
    return ret;
}

bool js_parser::parse_object(void* const parent, LPCXSTR& s, LPCXSTR e)
{
    s = _json_shift_space(s, e);
    if (s >= e)
    {
        set_error(X_T("unclosed object."));
        return false;
    }
    if (*s == '}')
    {//empty object
        ++s;
        m_pWalker->end_object(parent);
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

        if (*s == '}')
        {
            ++s;
            m_pWalker->end_object(parent);
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
                m_pWalker->end_object(parent);
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

    if (m_pError == nullptr)
        set_error(X_T("missing value."));

    return false;
}

bool js_parser::parse_array(void* const parent, LPCXSTR& s, LPCXSTR e)
{
    s = _json_shift_space(s, e);
    if (s >= e)
    {
        set_error(X_T("unclosed array."));
        return false;
    }
    if (*s == ']')
    {//empty array
        ++s;
        m_pWalker->end_array(parent);
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

        if (*s == ']')
        {
            ++s;
            m_pWalker->end_array(parent);
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
                m_pWalker->end_array(parent);
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

    if (m_pError == nullptr)
        set_error(X_T("missing value."));

    return false;
}

bool js_parser::parse_string(LPCXSTR& psz, LPCXSTR e, int nEndChar)
{
    LPCXSTR s = psz;
    for (; s < e && *s != nEndChar; ++s)
    {
        if (*s == '\\') ++s;
    }

    if (s < e && (s - psz) <= (std::numeric_limits<int32_t>::max)())
    {
        m_pWalker->string(std::basic_string_view<XCHAR>{ psz, static_cast<size_t>(s - psz) });
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

int64_t js_parser::_parse_long(LPCXSTR& psz, LPCXSTR e, js_type& eType) noexcept
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

    int64_t lValue = _xcstoll(scanstart, (LPXSTR*)&s, _Radix);
    eType = js_type::Long | (js_type)((_Radix - 1) << 4);
    psz = s;

    return lValue;
}

#if JSON_ENABLE_SIMD_PARSER

bool js_parser::parse_double(LPCXSTR& psz, LPCXSTR e) noexcept
{
    auto result = simd_double_parser::parser(psz, e);
    if (std::get<1>(result) == simd_double_parser::parser_result::Double)
    {
        m_pWalker->number_float(std::get<0>(result).d);
        return true;
    }
    else if (std::get<1>(result) == simd_double_parser::parser_result::Long)
    {
        m_pWalker->number_integer(std::get<0>(result).l, js_type::DecimalLong);
        return true;
    }
    else
    {
        set_error(X_T("invalid number."));
        return false;
    }
}

#else

bool js_parser::parse_double(LPCXSTR& psz, LPCXSTR e) noexcept
{
    double dval;
    fast_double_parser::result_type result = fast_double_parser::parse_number_base<XCHAR, '.'>(psz, &dval, e);
    if (result == fast_double_parser::result_type::Double)
    {
        m_pWalker->number_float(dval);
        return true;
    }
    else if (result == fast_double_parser::result_type::Long)
    {
        m_pWalker->number_integer(reinterpret_cast<int64_t&>(dval), js_type::DecimalLong);
        return true;
    }
    else
    {
        set_error(X_T("invalid number."));
        return false;
    }
}

#endif

bool js_parser::parse_number(LPCXSTR& psz, LPCXSTR e) noexcept
{
    LPCXSTR s = psz;

    if (*s == '0' && s + 1 < e && s[1] != '.' && (s[1] | 32) != 'e')
    {//0开头，可能是0，十六进制，二进制，八进制
#if JSON_ENABLE_JSON5
        if (_json_hex_leader(s[1]))
        {//0x，十六进制
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
                else
                    i = (i | 32) - 'a' + 10;

                i64 = (i64 << 4) | i;
            }

            m_pWalker->number_integer(i64, js_type::HexLong);

            psz = s;
            return true;
        }
        if (_json_bin_leader(s[1]))
        {//0b，二进制
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

            m_pWalker->number_integer(i64, js_type::BinaryLong);

            psz = s;
            return true;
        }
        if (_json_is_oct(s[1]))
        {//0[1-7]，八进制
            s += 1;

            uint64_t i64 = 0;
            while (s < e && _json_is_oct(*s) && i64 < 0x7fffffffffffffffULL)
            {
                int32_t i = *s++;
                i = i - '0';

                i64 = (i64 << 3) | i;
            }

            m_pWalker->number_integer(i64, js_type::OctalLong);

            psz = s;
            return true;
        }
#endif
        if (_json_is_digit(s[1]))
        {//089
            set_error(X_T("numbers cannot have leading zeroes."));
            return false;
        }
    }

    return parse_double(psz, e);
}

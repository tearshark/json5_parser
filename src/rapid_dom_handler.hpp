
rapid_value_alloctor::rapid_value_alloctor(size_t nNumBatch/* = 4*/)
    : m_pNextAlloc(nullptr)
    , m_pEndAlloc(nullptr)
    , m_nAllocedCount(0)
    , m_pFirstNode(nullptr)
    , m_nNumBatch((std::max)(nNumBatch, (size_t)64))
{
}

rapid_value_alloctor::~rapid_value_alloctor()
{
    for (char* p = m_pFirstNode; p; )
    {
        char* t = *(char**)p;
        _aligned_free(p);
        p = t;
    }
}

void rapid_value_alloctor::clear(size_t nNumBatch)
{
    for (char* p = m_pFirstNode; p; )
    {
        char* t = *(char**)p;
        _aligned_free(p);
        p = t;
    }
    m_pNextAlloc = nullptr;
    m_pEndAlloc = nullptr;
    m_nAllocedCount = 0;
    m_pFirstNode = nullptr;
    m_nNumBatch = (std::max)(nNumBatch, (size_t)64);
}

rapid_value* rapid_value_alloctor::alloc()
{
    if (m_pNextAlloc >= m_pEndAlloc)
    {
        if (m_nAllocedCount > m_nNumBatch * 2)
            m_nNumBatch *= 2;

        size_t nNumAlloc = sizeof(rapid_value) * m_nNumBatch;

        char* pNode = (char*)_aligned_malloc(nNumAlloc, alignof(rapid_value));
        if (pNode == nullptr)
            return nullptr;

        *(char**)pNode = m_pFirstNode;
        m_pFirstNode = pNode;

        m_pNextAlloc = (rapid_value*)(pNode + sizeof(rapid_value));
        m_pEndAlloc = m_pNextAlloc + m_nNumBatch - 1;
    }

    rapid_value* t = m_pNextAlloc;
    ++m_pNextAlloc;
    ++m_nAllocedCount;

    return t;
}



rapid_dom_handler::rapid_dom_handler(size_t nNunBatch)
    : m_Alloctor(nNunBatch)
    , m_pRootValue(nullptr)
{
}

rapid_dom_handler::~rapid_dom_handler()
{
}

inline void rapid_dom_handler::SetNameIf(rapid_value* v) noexcept
{
    if (m_pRootValue->type == JSON_Type::Object)
    {
        v->nlen = static_cast<uint16_t>(m_ChildName.size());
        v->name = m_ChildName.data();
    }
    else
    {
        v->nlen = 0;
        v->name = nullptr;
    }

    v->prev = m_pRootValue->elements;
    m_pRootValue->elements = v;
}

void* rapid_dom_handler::start_object(bool root)
{
    rapid_value* v = m_Alloctor.alloc();
    v->type = JSON_Type::Object;
    v->elements = nullptr;

    if (!root)
    {
        SetNameIf(v);
        void* ret = m_pRootValue;
        m_pRootValue = v;
        return ret;
    }
    else
    {
        v->nlen = 0;
        v->name = nullptr;
        m_pRootValue = v;
        return v;
    }
}

void rapid_dom_handler::end_object(void* v)
{
    m_pRootValue = reinterpret_cast<rapid_value*>(v);
}

void* rapid_dom_handler::start_array(bool root)
{
    rapid_value* v = m_Alloctor.alloc();
    v->type = JSON_Type::Array;
    v->elements = nullptr;

    if (!root)
    {
        SetNameIf(v);
        void* ret = m_pRootValue;
        m_pRootValue = v;
        return ret;
    }
    else
    {
        v->nlen = 0;
        v->name = nullptr;
        m_pRootValue = v;
        return v;
    }
}

void rapid_dom_handler::end_array(void* v)
{
    m_pRootValue = reinterpret_cast<rapid_value*>(v);
}

void rapid_dom_handler::null()
{
    rapid_value* v = m_Alloctor.alloc();
    v->type = JSON_Type::Nullptr;
    SetNameIf(v);
}

void rapid_dom_handler::string(const std::basic_string_view<XCHAR> str)
{
    rapid_value* v = m_Alloctor.alloc();
    v->type = JSON_Type::String;
    v->slen = static_cast<uint32_t>(str.size());
    v->str = str.data();
    SetNameIf(v);
}

void rapid_dom_handler::number_float(double value)
{
    rapid_value* v = m_Alloctor.alloc();
    v->type = JSON_Type::Double;
    v->f = value;
    SetNameIf(v);
}

void rapid_dom_handler::number_integer(int64_t value, JSON_Type ty)
{
    rapid_value* v = m_Alloctor.alloc();
    v->type = ty;
    v->l = value;
    SetNameIf(v);
}

void rapid_dom_handler::boolean(bool value)
{
    rapid_value* v = m_Alloctor.alloc();
    v->type = JSON_Type::Boolean;
    v->l = value ? 1 : 0;
    SetNameIf(v);
}

void rapid_dom_handler::key(const std::basic_string_view<XCHAR> name)
{
    m_ChildName = name;
}

void rapid_dom_handler::parse_error(LPCXSTR err, LPCXSTR stoped)
{
    m_pRootValue = nullptr;
    if (ErrorReport)
        ErrorReport(err, stoped);
}

intptr_t rapid_dom_handler::compact_mode(intptr_t mode)
{
    return mode;
}

LPXSTR JSON_LoadString(LPXSTR pszStart, LPCXSTR s, LPCXSTR e) noexcept
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
                            if constexpr (sizeof(*psz) == sizeof(char))
                            {
                                char buffer[8];
#ifdef _MSC_VER
                                int nCvt = sizeof(buffer);
                                wctomb_s(&nCvt, buffer, sizeof(buffer), wc);
#else
                                int nCvt = wctomb(buffer, wc);
#endif
                                for (int x = 0; x < nCvt; ++x)
                                    *psz++ = buffer[x];
                            }
                            else if constexpr (sizeof(*psz) == sizeof(char16_t))
                            {
                                *psz++ = wc;
                                if (wc > 0xffffu)
                                    *psz++ = wc >> 16;
                            }
                            else if constexpr (sizeof(*psz) == sizeof(char32_t))
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

std::basic_string<XCHAR> rapid_value::GetName() const
{
    std::basic_string<XCHAR> ret;

    ret.resize(this->nlen);
    auto e = JSON_LoadString(const_cast<XCHAR*>(ret.data()), this->name, this->name + this->nlen);
    assert(e - ret.data() <= this->nlen);
    *e = 0;
    ret.resize(e - ret.data());

    return ret;
}

std::basic_string<XCHAR> rapid_value::GetString() const
{
    std::basic_string<XCHAR> ret;

    if (this->type == JSON_Type::String)
    {
        ret.resize(this->slen);
        auto e = JSON_LoadString(const_cast<XCHAR*>(ret.data()), this->str, this->str + this->slen);
        assert(e - ret.data() <= (intptr_t)this->slen);
        *e = 0;
        ret.resize(e - ret.data());
    }

    return ret;
}

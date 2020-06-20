
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
	for (char* p = m_pFirstNode; p; )
	{
		char* t = *(char**)p;
		_aligned_free(p);
		p = t;
	}
}

void JSON_Alloctor::clear(size_t nNumBatch)
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

JSON_Value* JSON_Alloctor::alloc()
{
	if (m_pNextAlloc >= m_pEndAlloc)
	{
		if (m_nAllocedCount > m_nNumBatch * 2)
			m_nNumBatch *= 2;

		size_t nNumAlloc = sizeof(JSON_Value) * m_nNumBatch;

		char* pNode = (char*)_aligned_malloc(nNumAlloc, alignof(JSON_Value));

		*(char**)pNode = m_pFirstNode;
		m_pFirstNode = pNode;

		m_pNextAlloc = (JSON_Value*)(pNode + sizeof(JSON_Value));
		m_pEndAlloc = m_pNextAlloc + m_nNumBatch - 1;
	}

	JSON_Value* t = m_pNextAlloc;
	++m_pNextAlloc;
	++m_nAllocedCount;

	return t;
}



SAX_DOMHandler::SAX_DOMHandler(size_t nNunBatch)
	: m_Alloctor(nNunBatch)
	, m_pRootValue(nullptr)
{
}

SAX_DOMHandler::~SAX_DOMHandler()
{
}

inline void SAX_DOMHandler::SetNameIf(JSON_Value* v) noexcept
{
	if (m_pRootValue->type == JSON_Type::Object)
	{
		v->nlen = static_cast<uint16_t>(m_ChildName.end - m_ChildName.start);
		v->name = m_ChildName.start;
	}
	else
	{
		v->nlen = 0;
		v->name = nullptr;
	}

	v->prev = m_pRootValue->elements;
	m_pRootValue->elements = v;
}

void* SAX_DOMHandler::start_object(bool root)
{
	JSON_Value* v = m_Alloctor.alloc();
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

void SAX_DOMHandler::end_object(void* v)
{
	m_pRootValue = reinterpret_cast<JSON_Value*>(v);
}

void* SAX_DOMHandler::start_array(bool root)
{
	JSON_Value* v = m_Alloctor.alloc();
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

void SAX_DOMHandler::end_array(void* v)
{
	m_pRootValue = reinterpret_cast<JSON_Value*>(v);
}

void SAX_DOMHandler::null()
{
	JSON_Value* v = m_Alloctor.alloc();
	v->type = JSON_Type::Nullptr;
	SetNameIf(v);
}

void SAX_DOMHandler::string(JSON_String str)
{
	JSON_Value* v = m_Alloctor.alloc();
	v->type = JSON_Type::String;
	v->slen = static_cast<uint32_t>(str.end - str.start);
	v->str = str.start;
	SetNameIf(v);
}

void SAX_DOMHandler::number_float(double value)
{
	JSON_Value* v = m_Alloctor.alloc();
	v->type = JSON_Type::Double;
	v->f = value;
	SetNameIf(v);
}

void SAX_DOMHandler::number_integer(JSON_Type type, int64_t value)
{
	JSON_Value* v = m_Alloctor.alloc();
	v->type = type;
	v->l = value;
	SetNameIf(v);
}

void SAX_DOMHandler::boolean(bool value)
{
	JSON_Value* v = m_Alloctor.alloc();
	v->type = JSON_Type::Boolean;
	v->l = value ? 1 : 0;
	SetNameIf(v);
}

void SAX_DOMHandler::key(JSON_String name)
{
	m_ChildName = name;
}

void SAX_DOMHandler::parse_error(LPCXSTR err, LPCXSTR stoped)
{
	m_pRootValue = nullptr;
	if (ErrorReport)
		ErrorReport(err, stoped);
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
							if (sizeof(*psz) == sizeof(char))
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

std::basic_string<XCHAR> JSON_Value::GetName() const
{
	std::basic_string<XCHAR> ret;

	ret.resize(this->nlen);
	auto e = JSON_LoadString(const_cast<XCHAR*>(ret.data()), this->name, this->name + this->nlen);
	assert(e - ret.data() <= this->nlen);
	*e = 0;
	ret.resize(e - ret.data());

	return ret;
}

std::basic_string<XCHAR> JSON_Value::GetString() const
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


struct alignas(32) JSON_Value
{
	JSON_Type				type;
	uint16_t				nlen;			//Length of name.
	uint32_t				slen;			//Length of str, when type equal JSON_Type::String.

	union alignas(8)
	{
		JSON_Value*			elements;		//Last child, when type equal JSON_Type::Object or JSON_Type::Array. 
											//This means that the order of children is reversed.
		LPCXSTR				str;			//Valid when type equal JSON_Type::String.
		double				f;				//Valid when type equal JSON_Type::Double.
		int64_t				l;				//Valid when type equal JSON_Type::Long.
		int32_t				i;				//Valid when type equal JSON_Type::Boolean.
	};
	
	alignas(8)
	LPCXSTR 				name;			//Valid when the parent is an object.

	alignas(8)
	JSON_Value* 			prev;			//Previous sibling node, when the parent is an object or an array.
};

inline JSON_Type JSON_GetType(const JSON_Value* jv)
{
	return (JSON_Type)(jv->type & JSON_Type::LONG_MASK);
}

size_t JSON_ElementsCount(const JSON_Value* jv);
LPXSTR JSON_LoadString(LPXSTR pszStart, LPCXSTR s, LPCXSTR e) noexcept;
std::basic_string<XCHAR> JSON_GetName(const JSON_Value* jv);
std::basic_string<XCHAR> JSON_GetString(const JSON_Value* jv);


template<class _Vistor>
void JSON_ForeachElements(const JSON_Value* jv, const _Vistor& vistor)
{
	if (JSON_GetType(jv) == JSON_Type::Object || JSON_GetType(jv) == JSON_Type::Array)
		for (const JSON_Value* e = jv->elements; e != nullptr; e = e->prev)
		{
			vistor(e);
		}
}

template<class _Vistor>
void JSON_Vistor(const JSON_Value* jv, const _Vistor& vistor, const JSON_Value* jparent = nullptr)
{
	vistor(jv, jparent);

	if (JSON_GetType(jv) == JSON_Type::Object || JSON_GetType(jv) == JSON_Type::Array)
		for (const JSON_Value* e = jv->elements; e != nullptr; e = e->prev)
		{
			JSON_Vistor(e, vistor, jv);
		}
}

struct JSON_Alloctor
{
	JSON_Value * alloc();
	void clear(size_t nNumBatch);
	size_t size() const noexcept
	{
		return m_nAllocedCount;
	}

	JSON_Alloctor(size_t nNumBatch = 4);
	~JSON_Alloctor();
private:
	JSON_Value *		m_pNextAlloc;
	JSON_Value *		m_pEndAlloc;
	size_t				m_nAllocedCount;
	char *				m_pFirstNode;
	size_t				m_nNumBatch;
};

struct JSON_DOMWalker : public JSON_Walker
{
	JSON_DOMWalker(size_t nNunBatch);

	const JSON_Value* Value() const  noexcept { return m_pRootValue; }
	size_t Count() const  noexcept { return m_Alloctor.size(); }
private:
	JSON_Alloctor		m_Alloctor;
	JSON_Value*			m_pRootValue;
};

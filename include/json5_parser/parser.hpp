
struct alignas(32) JSON_Value
{
	JSON_Type				type;
	uint16_t				nlen;
	uint32_t				slen;

	union alignas(8)
	{
		JSON_Value*			elements;
		LPCXSTR				str;
		double				f;
		int64_t				l;
		int32_t				i;
	};

	LPCXSTR alignas(8)		name;

	JSON_Value* alignas(8)	next;

	struct alignas(sizeof(void*) * 2) Name
	{
		LPCXSTR			start;
		LPCXSTR			end;
	};
};

inline JSON_Type JSON_GetType(const JSON_Value* jv)
{
	return (JSON_Type)(jv->type & JSON_Type::JSONT_LONG_MASK);
}

VFX_API size_t JSON_ElementsCount(const JSON_Value& jv);

template<class _Vistor>
void JSON_ForeachElements(const JSON_Value* jv, const _Vistor& vistor)
{
	if (JSON_GetType(jv) == JSON_Type::JSONT_Object || JSON_GetType(jv) == JSON_Type::JSONT_Array)
		for (const JSON_Value* e = jv.elements; e != nullptr; e = e->next)
		{
			vistor(e);
		}
}

template<class _Vistor>
void JSON_Vistor(const JSON_Value* jv, const _Vistor& vistor, const JSON_Value* jparent = nullptr)
{
	vistor(jv, jparent);

	switch (jv->type & JSONT_LONG_MASK)
	{
	case JSON_Type::JSONT_Object:
	case JSON_Type::JSONT_Array:
		for (const JSON_Value* e = jv.elements; e != nullptr; e = e->next)
		{
			JSON_Vistor(e, vistor, jv);
		}
		break;
	}
}

struct JSON_Alloctor
{
	JSON_Value * alloc();
	void clear(size_t nNumBatch);
	size_t size() const
	{
		return m_nAllocedCount;
	}

	VFX_API JSON_Alloctor(size_t nNumBatch = 4);
	VFX_API ~JSON_Alloctor();
private:
	JSON_Value *		m_pNextAlloc;
	JSON_Value *		m_pEndAlloc;
	size_t				m_nAllocedCount;
	char *				m_pFirstNode;
	size_t				m_nNumBatch;
};

/*
object => { members }
members => pair , members
pair => string : value
array => [ elements ]
elements => value , elements
value => string | number | object | array
number => long | double
double => int.int
long  => binary | octal | decimal | hex | true | false | null
*/

struct JSON_Parser
{
	/*
	分析JSON格式的字符串
	在分析完毕后，JSON_Value里存的地址仍然是参数psz指向的内存快
	因此，要使用分析后的结果，需要保持psz仍然是有效的
	JSON_Parse类会保存参数psz指针，但不管理psz内存的释放
	*/
	VFX_API JSON_Value * Parse(size_t nNunBatch, LPCXSTR psz, LPCXSTR * ppszEnd = NULL);

	const JSON_Value * GetRootValue() const { return m_pRootValue; }
	size_t AllocedCount() const { return m_Alloctor.size(); }

	VFX_API JSON_Parser();
	VFX_API ~JSON_Parser();

	VFX_API static int64_t _parse_long(LPCXSTR& psz, LPCXSTR e, JSON_Type& eType);
private:
	JSON_Value * parse_start(LPCXSTR& s, LPCXSTR e);
	JSON_Value * parse_pair(LPCXSTR& s, LPCXSTR e);
	JSON_Value * parse_value(LPCXSTR& s, LPCXSTR e);
	JSON_Value * parse_object(JSON_Value* parent, LPCXSTR& s, LPCXSTR e);
	JSON_Value * parse_array(JSON_Value* parent, LPCXSTR& s, LPCXSTR e);
	JSON_Value * parse_string(LPCXSTR& s, LPCXSTR e, int nEndChar);
	JSON_Value * parse_number(LPCXSTR& s, LPCXSTR e);

	JSON_Alloctor		m_Alloctor;
	JSON_Value *		m_pRootValue;
};

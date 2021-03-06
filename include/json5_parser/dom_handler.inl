﻿
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


	JSON_Type GetType() const noexcept
	{
		return (JSON_Type)(this->type & JSON_Type::LONG_MASK);
	}

	JSON5_API size_t ElementsCount() const noexcept;
	JSON5_API std::basic_string<XCHAR> GetName() const;
	JSON5_API std::basic_string<XCHAR> GetString() const;

	template<class _Vistor>
	void ForeachElements(const _Vistor& vistor) const
	{
		if (this->type == JSON_Type::Object || this->type == JSON_Type::Array)
			for (const JSON_Value* e = this->elements; e != nullptr; e = e->prev)
			{
				vistor(e);
			}
	}

	template<class _Vistor>
	void Vistor(const _Vistor& vistor, const JSON_Value* jparent = nullptr) const
	{
		vistor(this, jparent);

		if (this->type == JSON_Type::Object || this->type == JSON_Type::Array)
			for (const JSON_Value* e = this->elements; e != nullptr; e = e->prev)
			{
				e->Vistor(vistor, this);
			}
	}
};

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

struct SAX_DOMHandler : public SAX_Handler
{
	std::function<void(LPCXSTR err, LPCXSTR stoped)> ErrorReport;

	JSON5_API SAX_DOMHandler(size_t nNunBatch);
	JSON5_API ~SAX_DOMHandler();

	const JSON_Value* Value() const  noexcept { return m_pRootValue; }
	size_t Count() const  noexcept { return m_Alloctor.size(); }

	virtual void number_integer(JSON_Type type, int64_t value) override;
	virtual void number_float(double value) override;
	virtual void string(const std::basic_string_view<XCHAR> str) override;
	virtual void boolean(bool value) override;
	virtual void* start_object(bool root) override;
	virtual void end_object(void*) override;
	virtual void* start_array(bool root) override;
	virtual void end_array(void*) override;
	virtual void null() override;

	virtual void key(const std::basic_string_view<XCHAR> name) override;

	virtual void parse_error(LPCXSTR err, LPCXSTR stoped) override;

private:
	JSON_Alloctor					m_Alloctor;
	JSON_Value*						m_pRootValue;
	std::basic_string_view<XCHAR>	m_ChildName;

	void SetNameIf(JSON_Value* v) noexcept;
};

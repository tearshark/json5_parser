
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

	size_t ElementsCount() const noexcept;
	std::basic_string<XCHAR> GetName() const;
	std::basic_string<XCHAR> GetString() const;

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

struct JSON_DOMWalker : public JSON_Walker
{
	std::function<void(LPCXSTR err, LPCXSTR stoped)> ErrorReport;

	JSON_DOMWalker(size_t nNunBatch);
	~JSON_DOMWalker();

	const JSON_Value* Value() const  noexcept { return m_pRootValue; }
	size_t Count() const  noexcept { return m_Alloctor.size(); }

	virtual void PushNull() override;
	virtual void* PushObject(bool root) override;
	virtual void* PushArray(bool root) override;
	virtual void PushString(JSON_String str) override;
	virtual void PushDouble(double value) override;
	virtual void PushLong(JSON_Type type, int64_t value) override;
	virtual void PushBoolean(bool value) override;
	virtual void PushName(JSON_String name) override;
	virtual void PopObject(void*) override;
	virtual void PopArray(void*) override;
	virtual void ErrorStop(LPCXSTR err, LPCXSTR stoped) override;

private:
	JSON_Alloctor		m_Alloctor;
	JSON_Value*			m_pRootValue;
	JSON_String			m_ChildName;

	void SetNameIf(JSON_Value* v);
};

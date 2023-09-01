
struct alignas(32) rapid_value
{
    js_type               type;
    uint16_t                nlen;           //Length of name.
    uint32_t                slen;           //Length of str, when type equal js_type::String.

    union alignas(8)
    {
        rapid_value* elements;              //Last child, when type equal js_type::Object or js_type::Array. 
                                            //This means that the order of children is reversed.
        LPCXSTR             str;            //Valid when type equal js_type::String.
        double              f;              //Valid when type equal js_type::Double.
        int64_t             l;              //Valid when type equal js_type::Long.
        int32_t             i;              //Valid when type equal js_type::Boolean.
    };

    alignas(8) LPCXSTR      name;           //Valid when the parent is an object.
    alignas(8) rapid_value* prev;           //Previous sibling node, when the parent is an object or an array.


    js_type getType() const noexcept
    {
        return (js_type)(this->type & js_type::LONG_MASK);
    }

    JSON5_API size_t elementsCount() const noexcept;
    JSON5_API std::basic_string<XCHAR> getName() const;
    JSON5_API std::basic_string<XCHAR> getString() const;

    template<class _Vistor>
    void foreachElements(const _Vistor& vistor__) const
    {
        if (this->type == js_type::Object || this->type == js_type::Array)
            for (const rapid_value* e = this->elements; e != nullptr; e = e->prev)
            {
                vistor__(e);
            }
    }

    template<class _Vistor>
    void vistor(const _Vistor& vistor__, const rapid_value* jparent = nullptr) const
    {
        vistor__(this, jparent);

        if (this->type == js_type::Object || this->type == js_type::Array)
            for (const rapid_value* e = this->elements; e != nullptr; e = e->prev)
            {
                e->vistor(vistor__, this);
            }
    }
};

struct rapid_value_alloctor
{
    rapid_value* alloc();
    void clear(size_t nNumBatch);
    size_t size() const noexcept
    {
        return m_nAllocedCount;
    }

    rapid_value_alloctor(size_t nNumBatch = 4);
    ~rapid_value_alloctor();
private:
    rapid_value*    m_pNextAlloc;
    rapid_value*    m_pEndAlloc;
    size_t          m_nAllocedCount;
    char*           m_pFirstNode;
    size_t          m_nNumBatch;
};

struct rapid_dom_handler : public js_sax_handler
{
    std::function<void(LPCXSTR err, LPCXSTR stoped)> ErrorReport;

    JSON5_API rapid_dom_handler(size_t nNunBatch);
    JSON5_API ~rapid_dom_handler();

    const rapid_value* value() const  noexcept { return m_pRootValue; }
    size_t count() const  noexcept { return m_Alloctor.size(); }

    virtual void number_integer(int64_t value, js_type type = js_type::Long) override;
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

    virtual intptr_t compact_mode(intptr_t mode) override;
private:
    rapid_value_alloctor            m_Alloctor;
    rapid_value*                    m_pRootValue;
    std::basic_string_view<XCHAR>   m_ChildName;

    void SetNameIf(rapid_value* v) noexcept;
};

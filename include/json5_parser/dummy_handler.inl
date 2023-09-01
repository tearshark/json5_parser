
struct js_dummy_handler : public js_sax_handler
{
    virtual void number_integer(int64_t, js_type = js_type::Long) override {}
    virtual void number_float(double) override {}
    virtual void string(const std::basic_string_view<XCHAR>) override {}
    virtual void boolean(bool) override {}
    virtual void* start_object(bool) override { return nullptr; }
    virtual void end_object(void*) override {}
    virtual void* start_array(bool) override { return nullptr; }
    virtual void end_array(void*) override {}
    virtual void null() override {}

    virtual void key(const std::basic_string_view<XCHAR>) override {}

    virtual void parse_error(LPCXSTR, LPCXSTR) override {}

    virtual intptr_t compact_mode(intptr_t) override { return 0; }
};

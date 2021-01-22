
struct SAX_Handler
{
	virtual void number_integer(JSON_Type type, int64_t value) = 0;
			void number_integer(int64_t value) {
				number_integer(JSON_Type::Long, value);
			}
	virtual void number_float(double value) = 0;
	virtual void string(const std::basic_string_view<XCHAR> str) = 0;
	virtual void boolean(bool value) = 0;
	virtual void* start_object(bool root) = 0;
	virtual void end_object(void*) = 0;
	virtual void* start_array(bool root) = 0;
	virtual void end_array(void*) = 0;
	virtual void null() = 0;

	virtual void key(const std::basic_string_view<XCHAR> name) = 0;

	virtual void parse_error(LPCXSTR err, LPCXSTR stoped) = 0;
};

struct SAX_DummyHandler : public SAX_Handler
{
	virtual void number_integer(JSON_Type , int64_t) override {}
	virtual void number_float(double ) override {}
	virtual void string(const std::basic_string_view<XCHAR>) override {}
	virtual void boolean(bool ) override {}
	virtual void* start_object(bool) override { return nullptr; }
	virtual void end_object(void*) override {}
	virtual void* start_array(bool) override { return nullptr; }
	virtual void end_array(void*) override {}
	virtual void null() override {}

	virtual void key(const std::basic_string_view<XCHAR>) override {}

	virtual void parse_error(LPCXSTR , LPCXSTR ) override {}
};
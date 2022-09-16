
struct js_sax_handler
{
	virtual void number_integer(int64_t value, JSON_Type type = JSON_Type::Long) = 0;
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

	virtual intptr_t compact_mode(intptr_t mode) = 0;
};


struct JSON_String
{
	LPCXSTR				start;
	LPCXSTR				end;
};

struct SAX_Handler
{
	virtual void number_integer(JSON_Type type, int64_t value) = 0;
	virtual void number_float(double value) = 0;
	virtual void string(JSON_String str) = 0;
	virtual void boolean(bool value) = 0;
	virtual void* start_object(bool root) = 0;
	virtual void end_object(void*) = 0;
	virtual void* start_array(bool root) = 0;
	virtual void end_array(void*) = 0;
	virtual void null() = 0;

	virtual void key(JSON_String name) = 0;

	virtual void parse_error(LPCXSTR err, LPCXSTR stoped) = 0;
};

struct SAX_DummyHandler : public SAX_Handler
{
	virtual void number_integer(JSON_Type , int64_t) override {}
	virtual void number_float(double ) override {}
	virtual void string(JSON_String ) override {}
	virtual void boolean(bool ) override {}
	virtual void* start_object(bool) override { return nullptr; }
	virtual void end_object(void*) override {}
	virtual void* start_array(bool) override { return nullptr; }
	virtual void end_array(void*) override {}
	virtual void null() override {}

	virtual void key(JSON_String ) override {}

	virtual void parse_error(LPCXSTR , LPCXSTR ) override {}
};
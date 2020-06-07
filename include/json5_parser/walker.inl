
struct JSON_String
{
	LPCXSTR				start;
	LPCXSTR				end;
};

struct JSON_Walker
{
	virtual void PushNull() = 0;
	virtual void* PushObject() = 0;
	virtual void* PushArray() = 0;
	virtual void PushString(JSON_String str) = 0;
	virtual void PushDouble(double value) = 0;
	virtual void PushLong(JSON_Type type, int64_t value) = 0;
	virtual void PushBoolean(bool value) = 0;

	virtual void PushName(JSON_String name) = 0;

	virtual void PopObject(void*) = 0;
	virtual void PopArray(void*) = 0;

	virtual void ErrorStop(LPCXSTR err, LPCXSTR stoped) = 0;
};

struct JSON_DummyWalker : public JSON_Walker
{
	virtual void PushNull() override {}
	virtual void* PushObject() override { return nullptr; }
	virtual void* PushArray() override { return nullptr; }
	virtual void PushString(JSON_String str) override {}
	virtual void PushDouble(double) override {}
	virtual void PushLong(JSON_Type, int64_t) override {}
	virtual void PushBoolean(bool) override {}

	virtual void PushName(JSON_String) override {}

	virtual void PopObject(void*) override {}
	virtual void PopArray(void*) override {}

	virtual void ErrorStop(LPCXSTR err, LPCXSTR stoped) override {}
};
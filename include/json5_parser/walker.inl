
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

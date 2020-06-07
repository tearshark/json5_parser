
struct JSON_DebugWalker : public JSON_Walker
{
	std::function<void(LPCXSTR err, LPCXSTR stoped)> ErrorReport;

	virtual void PushNull() override
	{
		std::cout << "null, " << std::endl;
	}
	virtual void* PushObject() override
	{
		std::cout << "{" << std::endl;
		return nullptr;
	}
	virtual void* PushArray() override
	{
		std::cout << "[" << std::endl;
		return nullptr;
	}
	virtual void PushString(JSON_String str) override
	{
		//std::cout << "\"" << std::basic_string_view<XCHAR>(str.start, str.end - str.start) << "\", " << std::endl;
	}
	virtual void PushDouble(double value) override
	{
		std::cout << value << ", " << std::endl;
	}
	virtual void PushLong(JSON_Type type, int64_t value) override
	{
		std::cout << value << ", " << std::endl;
	}
	virtual void PushBoolean(bool value) override
	{
		std::cout << (value ? "true" : "false") << ", " << std::endl;
	}

	virtual void PushName(JSON_String name) override
	{
		//std::cout << std::basic_string_view<XCHAR>(name.start, name.end - name.start) << ":" << std::endl;
	}

	virtual void PopObject(void*) override
	{
		std::cout << "}" << std::endl;
	}
	virtual void PopArray(void*) override
	{
		std::cout << "]" << std::endl;
	}

	virtual void ErrorStop(LPCXSTR err, LPCXSTR stoped) override
	{
		if (ErrorReport)
			ErrorReport(err, stoped);
	}
};
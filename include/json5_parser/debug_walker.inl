
namespace
{
	void print_string_view(std::string_view sv)
	{
		std::cout << sv;
	}
	void print_string_view(std::wstring_view sv)
	{
		std::wcout << sv;
	}
}

struct JSON_DebugWalker : public JSON_Walker
{
	std::function<void(LPCXSTR err, LPCXSTR stoped)> ErrorReport;

	virtual void PushNull() override
	{
		PrintTable();
		std::cout << "null," << std::endl;
	}
	virtual void* PushObject(bool) override
	{
		PrintTable();
		++tabCount;
		std::cout << "{" << std::endl;
		return nullptr;
	}
	virtual void* PushArray(bool) override
	{
		PrintTable();
		++tabCount;
		std::cout << "[" << std::endl;
		return nullptr;
	}
	virtual void PushString(JSON_String str) override
	{
		PrintTable();
		std::cout << "\"";
		print_string_view(std::basic_string_view<XCHAR>(str.start, str.end - str.start));
		std::cout << "\"," << std::endl;
	}
	virtual void PushDouble(double value) override
	{
		PrintTable();
		std::cout << value << "," << std::endl;
	}
	virtual void PushLong(JSON_Type type, int64_t value) override
	{
		PrintTable();
		std::cout << value << "," << std::endl;
	}
	virtual void PushBoolean(bool value) override
	{
		PrintTable();
		std::cout << (value ? "true" : "false") << "," << std::endl;
	}

	virtual void PushName(JSON_String name) override
	{
		PrintTable();
		std::cout << "\"";
		print_string_view(std::basic_string_view<XCHAR>(name.start, name.end - name.start));
		std::cout << "\":";

		cancelTab = true;
	}

	virtual void PopObject(void*) override
	{
		--tabCount;
		PrintTable();
		std::cout << "}," << std::endl;
	}
	virtual void PopArray(void*) override
	{
		--tabCount;
		PrintTable();
		std::cout << "]," << std::endl;
	}

	virtual void ErrorStop(LPCXSTR err, LPCXSTR stoped) override
	{
		if (ErrorReport)
			ErrorReport(err, stoped);
	}
private:
	void PrintTable()
	{
		if (tabCount > 0 && !cancelTab)
		{
			for (int i = 0; i < tabCount; ++i)
				std::cout << "  ";
		}
		cancelTab = false;
	}
	int tabCount = 0;
	bool cancelTab = false;
};
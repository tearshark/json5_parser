
namespace
{
    inline void print_string_view(std::string_view sv)
    {
        std::cout << sv;
    }
    inline void print_string_view(std::wstring_view sv)
    {
        std::wcout << sv;
    }
}

struct js_debug_handler : public js_sax_handler
{
    std::function<void(LPCXSTR err, LPCXSTR stoped)> ErrorReport;

    virtual void null() override
    {
        PrintTable();
        std::cout << "null," << std::endl;
    }
    virtual void* start_object(bool) override
    {
        PrintTable();
        ++tabCount;
        std::cout << "{" << std::endl;
        return nullptr;
    }
    virtual void* start_array(bool) override
    {
        PrintTable();
        ++tabCount;
        std::cout << "[" << std::endl;
        return nullptr;
    }
    virtual void string(const std::basic_string_view<XCHAR> str) override
    {
        PrintTable();
        std::cout << "\"";
        print_string_view(std::basic_string_view<XCHAR>(str.start, str.end - str.start));
        std::cout << "\"," << std::endl;
    }
    virtual void number_float(double value) override
    {
        PrintTable();
        std::cout << value << "," << std::endl;
    }
    virtual void number_integer(int64_t value, JSON_Type = JSON_Type::Long) override
    {
        PrintTable();
        std::cout << value << "," << std::endl;
    }
    virtual void boolean(bool value) override
    {
        PrintTable();
        std::cout << (value ? "true" : "false") << "," << std::endl;
    }

    virtual void key(const std::basic_string_view<XCHAR> name) override
    {
        PrintTable();
        std::cout << "\"";
        print_string_view(std::basic_string_view<XCHAR>(name.start, name.end - name.start));
        std::cout << "\":";

        cancelTab = true;
    }

    virtual void end_object(void*) override
    {
        --tabCount;
        PrintTable();
        std::cout << "}," << std::endl;
    }
    virtual void end_array(void*) override
    {
        --tabCount;
        PrintTable();
        std::cout << "]," << std::endl;
    }

    virtual void parse_error(LPCXSTR err, LPCXSTR stoped) override
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
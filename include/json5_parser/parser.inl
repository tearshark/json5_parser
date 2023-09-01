
/*
object => { members }
members => pair , members
pair => string : value
array => [ elements ]
elements => value , elements
value => string | number | object | array
number => long | double
double => int.int
long  => binary | octal | decimal | hex | true | false | null
*/

struct js_parser
{
    /*
    分析JSON格式的字符串
    要使用分析后的结果，需要保持psz仍然是有效的
    JSON_Parse类会保存参数psz指针，但不管理psz内存的释放
    */
    JSON5_API bool parse(js_sax_handler* handler, LPCXSTR psz, LPCXSTR* ppszEnd = nullptr);
    LPCXSTR errstr() const  noexcept { return m_pError; }

    JSON5_API static int64_t _parse_long(LPCXSTR& psz, LPCXSTR e, js_type& eType) noexcept;
private:
    bool parse_start(LPCXSTR& s, LPCXSTR e);
    bool parse_pair(LPCXSTR& s, LPCXSTR e);
    bool parse_value(LPCXSTR& s, LPCXSTR e);
    bool parse_object(void* const parent, LPCXSTR& s, LPCXSTR e);
    bool parse_array(void* const parent, LPCXSTR& s, LPCXSTR e);
    bool parse_string(LPCXSTR& s, LPCXSTR e, int nEndChar);
    bool parse_number(LPCXSTR& s, LPCXSTR e) noexcept;
    bool parse_double(LPCXSTR& s, LPCXSTR e) noexcept;

    LPCXSTR _json_collect_name(LPCXSTR _s, LPCXSTR _e, std::basic_string_view<XCHAR>& name) noexcept;
    void set_error(LPCXSTR e) noexcept;

    js_sax_handler* m_pWalker = nullptr;
    LPCXSTR         m_pError = nullptr;
};

JSON5_API LPXSTR JSON_LoadString(LPXSTR pszStart, LPCXSTR s, LPCXSTR e) noexcept;

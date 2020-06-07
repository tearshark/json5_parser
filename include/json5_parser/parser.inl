
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

struct JSON_Parser
{
	/*
	分析JSON格式的字符串
	在分析完毕后，JSON_Value里存的地址仍然是参数psz指向的内存快
	因此，要使用分析后的结果，需要保持psz仍然是有效的
	JSON_Parse类会保存参数psz指针，但不管理psz内存的释放
	*/
	bool Parse(JSON_Walker* walker, LPCXSTR psz, LPCXSTR * ppszEnd = nullptr);
	LPCXSTR Error() const  noexcept { return m_pError; }

	JSON_Parser();
	~JSON_Parser();

	static int64_t _parse_long(LPCXSTR& psz, LPCXSTR e, JSON_Type& eType) noexcept;
private:
	bool parse_start(LPCXSTR& s, LPCXSTR e);
	bool parse_pair(LPCXSTR& s, LPCXSTR e);
	bool parse_value(LPCXSTR& s, LPCXSTR e);
	bool parse_object(void* parent, LPCXSTR& s, LPCXSTR e);
	bool parse_array(void* parent, LPCXSTR& s, LPCXSTR e);
	bool parse_string(LPCXSTR& s, LPCXSTR e, int nEndChar);
	bool parse_number(LPCXSTR& s, LPCXSTR e) noexcept;

	LPCXSTR _json_collect_name(LPCXSTR _s, LPCXSTR _e, JSON_String& name) noexcept;
	void set_error(LPCXSTR e) noexcept;

	JSON_Walker*		m_pWalker;
	LPCXSTR				m_pError;
};

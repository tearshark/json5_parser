#include <iostream>
#include <chrono>
#include <memory>
#include <stdio.h>
#include <string.h>

#include "json5_parser/json5_parser.h"

const int N = 100;

namespace json = json5::singlebyte;

static void report_file_location(const char* path, const char* psz, const char* err)
{
	size_t line = 0, column = 0;
	for (; psz < err; ++psz)
	{
		if (*psz == '\r')
		{
			++line;
			column = 0;
		}

		if (*psz != '\n')
			column++;
	}

	std::cout << path << "(" << (line + 1) << ") col " << (column + 1);
}

std::unique_ptr<char[]> load_text_from_file(const char* path, size_t& length)
{
	FILE* file = fopen(path, "rb");
	if (file == nullptr)
		return nullptr;

	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* psz = new char[length + 1];
	const char* pszEnd = psz + length;
	fread(psz, 1, length, file);
	psz[length] = 0;
	fclose(file);

	return std::unique_ptr<char[]>{ psz };
}

template<class _Walker>
std::unique_ptr<char[]> load_json_from_file(_Walker& walker, const char* path)
{
	size_t length;
	std::unique_ptr<char[]> psz = load_text_from_file(path, length);
	if (psz != nullptr)
	{
		walker.ErrorReport = [path, psz = psz.get()](const char* err, const char* stoped)
		{
			char* newline = const_cast<char*>(strchr(stoped, '\r'));
			if (newline != nullptr) *newline = 0;

			report_file_location(path, psz, stoped);
			std::cout << " : error '" << err << "' at:" << std::endl << stoped << std::endl;
		};

		json::parser parser;

		const char* pszEnd = psz.get() + length;
		bool jv = parser.Parse(&walker, psz.get(), &pszEnd);
		if (jv == false)
			psz = nullptr;
	}

	return psz;
}

void json5_vistor(const char* path)
{
	json::walker walker{ 1024 };
	auto buffer = load_json_from_file(walker, path);
	if (buffer != nullptr && walker.Value() != nullptr)
	{
		walker.Value()->Vistor( 
			[](const json::value* js, const json::value* parent)
			{
				if (parent != nullptr && parent->GetType() == json5::JSON_Type::Object)
				{
					auto name = js->GetName();
					std::cout << "\"" << name << "\":";
				}

				switch (js->GetType())
				{
				case json5::JSON_Type::Nullptr:
					std::cout << "null";
					break;
				case json5::JSON_Type::String:
					std::cout << "\"" << js->GetString() << "\"";
					break;
				case json5::JSON_Type::Double:
					std::cout << js->f;
					break;
				case json5::JSON_Type::Long:
					std::cout << js->l;
					break;
				case json5::JSON_Type::Boolean:
					std::cout << (js->i ? "true" : "false");
					break;
				default:
					break;
				}

				std::cout << std::endl;
		});
	}
}

namespace json5
{
	void _json_print_char_flag();
}

int main(int argc, char* argv[])
{
	//json5::_json_print_char_flag();

	for(int i = 1; i < argc; ++i)
	{
		json5_vistor(argv[i]);
	}

	return 0;
}

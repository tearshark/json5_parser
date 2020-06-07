#include <iostream>
#include <chrono>
#include <memory>
#include <stdio.h>
#include <string.h>

#include "json5_parser/json5_parser.h"

const int N = 100;

using namespace json5;

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

template<class _Walker>
std::unique_ptr<char[]> load_json_from_file(_Walker& walker, const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file == nullptr)
		return nullptr;

	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* psz = new char[length + 1];
	const char* pszEnd = psz + length;
	fread(psz, 1, length, file);
	psz[length] = 0;
	fclose(file);

	json5::parser parser;
	walker.ErrorReport = [path, psz, pszEnd](const char* err, const char* stoped)
	{
		char* newline = (char*)strchr(pszEnd, '\r');
		if (newline != nullptr) *newline = 0;

		report_file_location(path, psz, pszEnd);
		std::cout << " : error '" << err << "' at:" << std::endl << pszEnd << std::endl;
	};

	auto jv = parser.Parse(&walker, psz, &pszEnd);
	if (jv == false)
	{
		delete[] psz;
		return nullptr;
	}

	return std::unique_ptr<char[]>{ psz };
}

void json5_vistor(const char* path)
{
	json5::singlebyte::JSON_DebugWalker walker;
	auto buffer = load_json_from_file(walker, path);
/*
	if (buffer)
	{
		singlebyte::JSON_Vistor(parser.Value(), 
			[](const json5::value* js, const json5::value* parent)
			{
				if (parent != nullptr && singlebyte::JSON_GetType(parent) == JSON_Type::Object)
				{
					auto name = singlebyte::JSON_GetName(js);
					std::cout << "\"" << name << "\":";
				}

				switch (singlebyte::JSON_GetType(js))
				{
				case JSON_Type::Nullptr:
					std::cout << "null";
					break;
				case JSON_Type::String:
					std::cout << "\"" << singlebyte::JSON_GetString(js) << "\"";
					break;
				case JSON_Type::Double:
					std::cout << js->f;
					break;
				case JSON_Type::Long:
					std::cout << js->l;
					break;
				case JSON_Type::Boolean:
					std::cout << (js->i ? "true" : "false");
					break;
				default:
					break;
				}

				std::cout << std::endl;
		});
	}
*/
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

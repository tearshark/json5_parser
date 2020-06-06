#include <iostream>
#include <chrono>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json5_parser/json5_parser.h"

const int N = 100;

using namespace json5;

static void report_file_location(const char* path, const wchar_t* psz, const wchar_t* err)
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

std::unique_ptr<wchar_t[]> load_json_from_file(json5::wparser& parser, const char* path)
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

	setlocale(LC_ALL, "en_US.UTF-8:en-US");
	size_t wlength = mbstowcs(nullptr, psz, length);
	wchar_t* pwsz = new wchar_t[wlength + 1];
	wlength = mbstowcs(pwsz, psz, length);
	pwsz[length] = 0;
	delete[] psz;
	const wchar_t* pwszEnd = pwsz + wlength;

	auto* jv = parser.Parse(1024, pwsz, &pwszEnd);
	if (parser.Error() != nullptr)
	{
		wchar_t* newline = (wchar_t*)wcschr(pwszEnd, '\r');
		if (newline != nullptr) *newline = 0;

		report_file_location(path, pwsz, pwszEnd);
		std::wcout << L" : error '" << parser.Error() << L"' at:" << std::endl << pwszEnd << std::endl;
	}
	
	if (jv == nullptr)
	{
		delete[] pwsz;
		return nullptr;
	}

	return std::unique_ptr<wchar_t[]>{ pwsz };
}

void json5_vistor(const char* path)
{
	json5::wparser parser;
	auto buffer = load_json_from_file(parser, path);
	if (buffer)
	{
		unicode::JSON_Vistor(parser.Value(), 
			[](const json5::wvalue* js, const json5::wvalue* parent)
			{
				if (parent != nullptr && unicode::JSON_GetType(parent) == JSON_Type::Object)
				{
					auto name = unicode::JSON_GetName(js);
					std::wcout << L"\"" << name << L"\":";
				}

				switch (unicode::JSON_GetType(js))
				{
				case JSON_Type::Nullptr:
					std::wcout << L"null";
					break;
				case JSON_Type::String:
					std::wcout << L"\"" << unicode::JSON_GetString(js) << L"\"";
					break;
				case JSON_Type::Double:
					std::wcout << js->f;
					break;
				case JSON_Type::Long:
					std::wcout << js->l;
					break;
				case JSON_Type::Boolean:
					std::wcout << (js->i ? L"true" : L"false");
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

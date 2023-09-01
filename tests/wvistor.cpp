#include <iostream>
#include <chrono>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json5_parser/json5_parser.h"

#pragma warning(disable : 4996)

namespace json = json5::unicode;

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

std::unique_ptr<wchar_t[]> load_text_from_file(const char* path, size_t& wlength)
{
	FILE* file = fopen(path, "rb");
	if (file == nullptr)
		return nullptr;

	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* psz = new char[length + 1];
	fread(psz, 1, length, file);
	psz[length] = 0;
	fclose(file);

	setlocale(LC_ALL, "en_US.UTF-8:en-US");
	wlength = mbstowcs(nullptr, psz, length);
	wchar_t* pwsz = new wchar_t[wlength + 1];
	(void)mbstowcs(pwsz, psz, length);
	pwsz[wlength] = 0;
	delete[] psz;

	return std::unique_ptr<wchar_t[]>{ pwsz };
}

template<class _SAX>
std::unique_ptr<wchar_t[]> load_json_from_file(_SAX& sax, const char* path)
{
	size_t length;
	std::unique_ptr<wchar_t[]> psz = load_text_from_file(path, length);
	if (psz)
	{
		sax.ErrorReport = [path, psz = psz.get()](const wchar_t* err, const wchar_t* stoped)
		{
			wchar_t* newline = const_cast<wchar_t*>(wcschr(stoped, '\r'));
			if (newline != nullptr) *newline = 0;

			report_file_location(path, psz, stoped);
			std::wcout << L" : error '" << err << L"' at:" << std::endl << stoped << std::endl;
		};

		json::js_parser parser;

		const wchar_t* pszEnd = psz.get() + length;
		bool jv = parser.parse(&sax, psz.get(), &pszEnd);
		if (jv == false)
			psz = nullptr;
	}

	return psz;
}

void json5_vistor(const char* path)
{
	json::rapid_dom_handler sax{ 1024 };
	auto buffer = load_json_from_file(sax, path);
	if (buffer != nullptr && sax.value() != nullptr)
	{
		sax.value()->vistor(
			[](const json::rapid_value* js, const json::rapid_value* parent)
			{
				if (parent != nullptr && parent->getType() == json5::js_type::Object)
				{
					auto name = js->getName();
					std::wcout << L"\"" << name << L"\":";
				}

				switch (js->getType())
				{
				case json5::js_type::Nullptr:
					std::wcout << L"null";
					break;
				case json5::js_type::String:
					std::wcout << L"\"" << js->getString() << L"\"";
					break;
				case json5::js_type::Double:
					std::wcout << js->f;
					break;
				case json5::js_type::Long:
					std::wcout << js->l;
					break;
				case json5::js_type::Boolean:
					std::wcout << (js->i ? L"true" : L"false");
					break;
				default:
					break;
				}

				std::cout << std::endl;
		});
	}
}

int main(int argc, char* argv[])
{
	for(int i = 1; i < argc; ++i)
	{
		json5_vistor(argv[i]);
	}

	return 0;
}

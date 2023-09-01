﻿#include <iostream>
#include <chrono>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <variant>
#include <map>
#include <vector>

#include "json5_parser/json5_parser.h"
#pragma warning(disable : 4996)

namespace json = json5::singlebyte;

static void report_file_location(const char* path, const char* psz, const char* err)
{
	size_t line = 0, column = 0;
	for (; psz < err; ++psz)
	{
		if (*psz == '\n')
		{
			++line;
			column = 0;
			continue;
		}

		if (*psz != '\r')
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
	fread(psz, 1, length, file);
	psz[length] = 0;
	fclose(file);

	return std::unique_ptr<char[]>{ psz };
}

template<class _SAX>
std::unique_ptr<char[]> load_json_from_file(_SAX& sax, const char* path)
{
	size_t length;
	std::unique_ptr<char[]> psz = load_text_from_file(path, length);
	if (psz != nullptr)
	{
		sax.ErrorReport = [path, psz = psz.get()](const char* err, const char* stoped)
		{
			char* newline = const_cast<char*>(strchr(stoped, '\n'));
			if (newline != nullptr)
			{
				if (newline - stoped > 80)
					newline = (char*)stoped + 80;
				*newline = 0;
			}

			report_file_location(path, psz, stoped);
			std::cout << " : error '" << err << "' at:" << std::endl << stoped << std::endl;
		};

		json::js_parser parser;

		const char* pszEnd = psz.get() + length;
		bool jv = parser.parse(&sax, psz.get(), &pszEnd);
		if (jv == false)
			psz = nullptr;
	}

	return psz;
}

void json5_vistor(const char* path)
{
	json::rapid_dom_handler dom{ 1024 };
	auto buffer = load_json_from_file(dom, path);
	if (buffer != nullptr && dom.value() != nullptr)
	{
		dom.value()->vistor(
			[](const json::rapid_value* js, const json::rapid_value* parent)
			{
				if (parent != nullptr && parent->getType() == json5::js_type::Object)
				{
					auto name = js->getName();
					std::cout << "\"" << name << "\":";
				}

				switch (js->getType())
				{
				case json5::js_type::Nullptr:
					std::cout << "null";
					break;
				case json5::js_type::String:
					std::cout << "\"" << js->getString() << "\"";
					break;
				case json5::js_type::Double:
					std::cout << js->f;
					break;
				case json5::js_type::Long:
					std::cout << js->l;
					break;
				case json5::js_type::Boolean:
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

struct js_value;
struct js_array;
struct js_object;

int main(int argc, char* argv[])
{

    using js_variant = std::variant<int64_t, double, std::string, bool, js_object*, js_array*, std::nullptr_t, std::monostate>;
	std::cout << sizeof(js_variant) << std::endl;

	//json5::_json_print_char_flag();

	for(int i = 1; i < argc; ++i)
	{
		json5_vistor(argv[i]);
	}

	return 0;
}

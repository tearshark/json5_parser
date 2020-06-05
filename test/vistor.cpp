#include <iostream>
#include <chrono>
#include <stdio.h>

#include "json5_parser/json5_parser.h"

const int N = 100;

using namespace json5;

std::unique_ptr<char[]> load_json_from_file(json5::parser& parser, const char* path)
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

	auto* jv = parser.Parse(1024, psz, &pszEnd);
	if (parser.Error() != nullptr)
	{
		char* newline = (char*)strchr(pszEnd, '\r');
		if (newline != nullptr) *newline = 0;

		std::cout << "error '" << parser.Error() << "' at:" << std::endl << pszEnd << std::endl;
	}
	
	if (jv == nullptr)
	{
		delete[] psz;
		return nullptr;
	}

	return std::unique_ptr<char[]>{ psz };
}

void json5_vistor(const char* path)
{
	json5::parser parser;
	auto buffer = load_json_from_file(parser, path);
	if (buffer)
	{
		singlebyte::JSON_Vistor(parser.Value(), 
			[](const json5::value* js, const json5::value* parent)
			{
				if (parent != nullptr && singlebyte::JSON_GetType(parent) == JSON_Type::JSONT_Object)
				{
					auto name = singlebyte::JSON_GetName(js);
					std::cout << "\"" << name << "\":";
				}

				switch (singlebyte::JSON_GetType(js))
				{
				case JSON_Type::JSONT_Nullptr:
					std::cout << "null";
					break;
				case JSON_Type::JSONT_String:
					std::cout << "\"" << singlebyte::JSON_GetString(js) << "\"";
					break;
				case JSON_Type::JSONT_Double:
					std::cout << js->f;
					break;
				case JSON_Type::JSONT_Long:
					std::cout << js->l;
					break;
				case JSON_Type::JSONT_Boolean:
					std::cout << js->i ? "true" : "false";
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

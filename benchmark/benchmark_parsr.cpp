#include <iostream>
#include <chrono>
#include <tuple>
#include <stdio.h>

#include "json5_parser/json5_parser.h"

#if __has_include("rapidjson/document.h")
#include "rapidjson/document.h"
#  define have_rapidjson 1
#else
#  define have_rapidjson 0
#endif

#pragma warning(disable : 4996)

namespace json = json5::singlebyte;

const int N = 800;

std::tuple<char*, size_t> load_from_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file != nullptr)
	{
		fseek(file, 0, SEEK_END);
		size_t length = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* psz = new char[length + 1];
		fread(psz, 1, length, file);
		psz[length] = 0;
		fclose(file);

		return { psz, length };
	}

	return { nullptr, 0 };
}

template<class _SAX, class... Args>
void benchmark_json5_parser(const char* path, Args&&... args)
{
	std::cout << __FUNCTION__ << " parse file: " << path << std::endl;

	char* psz;
	size_t length;
	std::tie(psz, length) = load_from_file(path);
	if (psz != nullptr)
	{
		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < N; ++i)
		{
			json::js_parser parser;
			_SAX sax(std::forward<Args>(args)...);

			const char* pszEnd = psz + length;
			bool jv = parser.Parse(&sax, psz, &pszEnd);
			if (jv == false)
			{
				std::cout << "failed." << std::endl;
				break;
			}
		}

		auto dt = std::chrono::high_resolution_clock::now() - start;
		auto second = std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();
		double speed = (length * N / second);
		std::cout << "parse " << length << " bytes cost " << second << " s, speeds is " << speed / (1024.0 * 1024.0) << " MB/s" << std::endl;

		delete[] psz;
	}
}

#if have_rapidjson
void benchmark_rapidjson(const char* path)
{
	std::cout << __FUNCTION__ << " parse file: " << path << std::endl;

	char* psz;
	size_t length;
	std::tie(psz, length) = load_from_file(path);
	if (psz != nullptr)
	{
		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < N; ++i)
		{
			rapidjson::Document js;
			if (js.Parse(psz, length).HasParseError())
			{
				std::cout << "failed." << std::endl;
				break;
			}
		}

		auto dt = std::chrono::high_resolution_clock::now() - start;
		auto second = std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();
		double speed = (length * N / second);
		std::cout << "parse " << length << " bytes cost " << second << " s, speeds is " << speed / (1024.0 * 1024.0) << " MB/s" << std::endl;

		delete[] psz;
	}
}
#endif

int main(int argc, char* argv[])
{
	std::cout << "dummy" << std::endl;
	for (int i = 1; i < argc; ++i)
	{
		benchmark_json5_parser<json5::singlebyte::js_dummy_handler>(argv[i]);
	}

	std::cout << std::endl;
	std::cout << "DOM" << std::endl;
	for (int i = 1; i < argc; ++i)
	{
		benchmark_json5_parser<json5::singlebyte::rapid_dom_handler>(argv[i], 1024);
	}

#if have_rapidjson
	std::cout << std::endl;
	for (int i = 1; i < argc; ++i)
	{
		benchmark_rapidjson(argv[i]);
	}
#endif

	return 0;
}

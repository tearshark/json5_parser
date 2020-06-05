#include <iostream>
#include <chrono>
#include <stdio.h>

#include "json5_parser/json5_parser.h"

const int N = 100;

void benchmark_parser_json(const char* pszPath)
{
	std::cout << __FUNCTION__ << " parse file: " << pszPath << std::endl;

	FILE* file = fopen(pszPath, "rb");
	if (file != nullptr)
	{
		fseek(file, 0, SEEK_END);
		size_t length = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* psz = new char[length + 1];
		const char* pszEnd = psz + length;
		fread(psz, 1, length, file);
		psz[length] = 0;

		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < N; ++i)
		{
			json5::parser parser;
			auto* jv = parser.Parse(1024, psz, &pszEnd);
			if (jv == nullptr)
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
		fclose(file);
	}
}

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		benchmark_parser_json(argv[i]);
	}
	return 0;
}

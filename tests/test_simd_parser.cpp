﻿#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cassert>

#pragma warning(disable : 4996 4101)
#include "json5_parser/simd_double_parser.h"
#include "json5_parser/fast_double_parser.h"

int main(int argc, char* argv[])
{
	const char* psz[] =
	{
		"0",
		"-0",
		"1",
		"-1",
		"1.2345678998",
		"1.23456789987",
		"1.234567899876",
		"1.2345678998765",
		"1.23456789987654",
		"1.234567899876543",
		"1.2345678998765432",
		"1.23456789987654321",
		"1.234567899876543210",
		"1.2345678998765432101",
		"1.23456789987654321012",
		"1.234567899876543210123",
		"0.12",
		"0.12E-309",
		"12345678998765432101.23E50",	//??
		"123456789987654321012.3E50",	//??
		"9223372036854775807",	//MAX 64bits int
		"9223372036854775808",
		"1E308",
		"12E308",
		"0.12E-309",
		"1234567899876543E-308",
		"123456789987654321E1",
		"1234567899876543210123E1",
		"1.234567899876543210123E50",
		"12.34567899876543210123E50",
		"123.4567899876543210123E50",
		"1234.567899876543210123E50",
		"12345.67899876543210123E50",
		"123456.7899876543210123E50",
		"1234567.899876543210123E50",
		"12345678.99876543210123E50",
		"123456789.9876543210123E50",
		"1234567899.876543210123E50",
		"12345678998.76543210123E50",
		"123456789987.6543210123E50",
		"1234567899876.543210123E50",
		"12345678998765.43210123E50",
		"123456789987654.3210123E50",
		"1234567899876543.210123E50",
		"12345678998765432.10123E50",
		"123456789987654321.0123E50",
		"1234567899876543210.123E50",
		"12345678998765432101.23E50",	//??
		"123456789987654321012.3E50",	//??
		"-1",
		"-12",
		"-1234567899876543",
		"-123456789987654321",
		"-9223372036854775807",	//MAX 64bits int
		"-9223372036854775808",
		"-1234567899876543210123",
		"-1.234567899876543210123",
		"-12.34567899876543210123",
		"-123.4567899876543210123",
		"-1234.567899876543210123",
		"-12345.67899876543210123",
		"-123456.7899876543210123",
		"-1234567.899876543210123",
		"-12345678.99876543210123",
		"-123456789.9876543210123",
		"-1234567899.876543210123",
		"-12345678998.76543210123",
		"-123456789987.6543210123",
		"-1234567899876.543210123",
		"-12345678998765.43210123",
		"-123456789987654.3210123",
		"-1234567899876543.210123",
		"-12345678998765432.10123",
		"-123456789987654321.0123",
		"-1234567899876543210.123",
		"-12345678998765432101.23",
		"-123456789987654321012.3",
		"+1",
		"+12",
		"+1234567899876543",
		"+123456789987654321",
		"+9223372036854775807",	//MAX 64bits int
		"+9223372036854775808",
		"+1234567899876543210123",
		"0.1234567899876543210123",
		"0.01234567899876543210123",
		"0.001234567899876543210123",
		"0.0001234567899876543210123",
		"0.00001234567899876543210123",
		"0.000001234567899876543210123",
		"0.0000001234567899876543210123",
		"0.00000001234567899876543210123",
		"0.000000001234567899876543210123",
		"0.0000000001234567899876543210123",
		"0.00000000001234567899876543210123",
		"0.000000000001234567899876543210123",
		"0.0000000000001234567899876543210123",
		"0.00000000000001234567899876543210123",
		"0.000000000000001234567899876543210123",
		"0.0000000000000001234567899876543210123",
		"0.00000000000000001234567899876543210123",
		"0.000000000000000001234567899876543210123",
		"0.0000000000000000001234567899876543210123",
		"0.00000000000000000001234567899876543210123",
		"0.000000000000000000001234567899876543210123",
	};

	simd_double_parser::number_value nv;
	simd_double_parser::parser_result isDouble;
	(void)isDouble;

	for (const char* s : psz)
	{
		const size_t l = strlen(s);
		const char* t = s;
/*
		std::tie(nv, isDouble) = simd_double_parser::parser(t, s + l);
		if (isDouble == simd_double_parser::parser_result::Double)
		{
			printf("'%s' is double, value=%.16lE\r\n", s, nv.d);

			double d2 = std::strtold(s, nullptr);
			char buff1[64];
			char buff2[64];
			sprintf(buff1, "%.15lE", nv.d);
			sprintf(buff2, "%.15lE", d2);
			if (strcmp(buff1, buff2) != 0)
				printf("Huge difference. 'strtold' is %s.\r\n", buff2);
		}
		else if (isDouble == simd_double_parser::parser_result::Long)
		{
			printf("'%s' is long, value=%" PRId64 "\r\n", s, nv.l);
		}
		else
		{
			printf("'%s' is invalid\r\n", s);
		}

		t = s;
*/
		auto result = fast_double_parser::parse_number_base<char, '.'>(t, &nv.d, s + l);
		if (result == fast_double_parser::result_type::Double)
		{
			printf("'%s' is double, value=%.16lE\r\n", s, nv.d);

			double d2 = std::strtold(s, nullptr);
			char buff1[64];
			char buff2[64];
			sprintf(buff1, "%.17lE", nv.d);
			sprintf(buff2, "%.17lE", d2);
			//if (strcmp(buff1, buff2) != 0)
			if (nv.d != d2)
				printf("Huge difference. 'strtold' is %s.\r\n", buff2);
		}
		else if (result == fast_double_parser::result_type::Long)
		{
			printf("'%s' is long, value=%" PRId64 "\r\n", s, nv.l);
		}
		else
		{
			printf("'%s' is invalid\r\n", s);
		}
	}

	return 0;
}
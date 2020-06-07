
LPXSTR JSON_LoadString(LPXSTR pszStart, LPCXSTR s, LPCXSTR e) noexcept
{
	LPXSTR psz = pszStart;
	for (; s < e; ++s)
	{
		if (*s == '\\')
		{
			++s;
			if (s < e)
			{
				switch (*s)
				{
				default: *psz++ = *s;  break;
				case 'b':*psz++ = '\b'; break;
				case 'f':*psz++ = '\f'; break;
				case 't':*psz++ = '\t'; break;
				case 'n':*psz++ = '\n'; break;
				case '\n':*psz++ = '\n'; break;
				case 'r':*psz++ = '\r'; break;
				case '\r':*psz++ = '\r'; break;
				case 'u':
					if (s + 4 < e)
					{
						++s;
						uint32_t wc = 0;
						for (size_t k = 0; k < 4; ++k)
						{
							uint32_t c = *s++;

							if (c >= '0' && c <= '9')
								c = c - '0';
							else if (c >= 'a' && c <= 'f')
								c = c - 'a' + 10;
							else if (c >= 'A' && c <= 'F')
								c = c - 'A' + 10;
							else
							{
								wc = 0;
								break;
							}

							wc = (wc << 4) | c;
						}
						--s;

						if (wc != 0)
						{
#pragma warning(disable : 4244)
							if (sizeof(*psz) == sizeof(char))
							{
								char buffer[8];
								//int nCvt = sizeof(buffer);
								//wctomb_s(&nCvt, buffer, sizeof(buffer), wc);
								int nCvt = wctomb(buffer, wc);
								for (int x = 0; x < nCvt; ++x)
									*psz++ = buffer[x];
							}
							else if (sizeof(*psz) == sizeof(char16_t))
							{
								*psz++ = wc;
								if (wc > 0xffffu)
									*psz++ = wc >> 16;
							}
							else if (sizeof(*psz) == sizeof(char32_t))
							{
								*psz++ = (char32_t)wc + 0x10000;
							}
							else
							{
								assert(false);
							}
#pragma warning(default : 4244)
						}
					}
					break;	//\u5efa\u7b51 TODO 处理Unicode的读取
				}
			}
			else
			{
				*psz++ = '\\';
			}
		}
		else
		{
			*psz++ = *s;
		}
	}

	return psz;
}

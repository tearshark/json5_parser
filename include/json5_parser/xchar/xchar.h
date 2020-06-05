
#define X_T		__X

namespace singlebyte
{
	typedef char			XSCHAR;
	typedef char			XCHAR;
	typedef unsigned char	XUCHAR;
	typedef const XCHAR*	LPCXSTR;
	typedef XCHAR*			LPXSTR;
}

namespace unicode
{
	typedef wchar_t			XSCHAR;
	typedef wchar_t			XCHAR;
	typedef wchar_t			XUCHAR;
	typedef const XCHAR*	LPCXSTR;
	typedef XCHAR*			LPXSTR;
}

namespace multibyte
{
	typedef char			XSCHAR;
	typedef unsigned char	XCHAR;
	typedef unsigned char	XUCHAR;
	typedef const XCHAR*	LPCXSTR;
	typedef XCHAR*			LPXSTR;
}

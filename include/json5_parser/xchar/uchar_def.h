#define _XEOF       WEOF
#define __X(x)      L ## x
#define X_UNICODE	1

/* Program */
#define _xmain      wmain
#define _xWinMain   wWinMain
#define _xenviron   _wenviron
#define __xargv     __wargv

/* Formatted i/o */
#define _xprintf    wprintf
#define _xcprintf   _cwprintf
#define _fxprintf   fwprintf
#define _sxprintf   swprintf
#define _scxprintf  _scwprintf
#define _snxprintf  _snwprintf
#define _snxprintf_s  _snwprintf_s
#define _vxprintf   vwprintf
#define _vfxprintf  vfwprintf
#define _vsxprintf  vswprintf
#define _vscxprintf _vscwprintf
#define _vsnxprintf _vsnwprintf
#define _xscanf     wscanf
#define _xcscanf    _cwscanf
#define _fxscanf    fwscanf
#define _sxscanf    swscanf
#define _snxscanf   _snwscanf

#if _MSC_FULL_VER >= 1400
#define _sxscanf_s  swscanf_s
#define _vsnxprintf_s _vsnwprintf_s
#else
#define _sxscanf_s  swscanf
#define _vsnxprintf_s(_DstBuf,_DstSize,_MaxCount,_Format,_ArgList) _vsnwprintf((_DstBuf),(_MaxCount),(_Format),(_ArgList))
#endif

/* Unformatted i/o */
#define _fgetxc     fgetwc
#define _fgetxchar  _fgetwchar
#define _fgetxs     fgetws
#define _fputxc     fputwc
#define _fputxchar  _fputwchar
#define _fputxs     fputws
#define _cputxs     _cputws
#define _cgetxs     _cgetws
#define _getxc      getwc
#define _getxch     _getwch
#define _getxche    _getwche
#define _getxchar   getwchar
#define _getxs      _getws
#define _putxc      putwc
#define _putxchar   putwchar
#define _putxch     _putwch
#define _putxs      _putws
#define _ungetxc    ungetwc
#define _ungetxch   _ungetwch

/* String conversion functions */
#define _xcstod     wcstod
#define _xcstol     wcstol
#define _xcstoul    wcstoul
#define _xstof      _wtof
#define _xstol      _wtol
#define _xstoi      _wtoi
#define _xstoi64    _wtoi64

#define _itox       _itow
#define _ltox       _ltow
#define _ultox      _ultow
#define _xtoi       _wtoi
#define _xtol       _wtol

#define _xtoi64     _wtoi64
#define _xcstoi64   _wcstoi64
#define _xcstoui64  _wcstoui64
#define _i64tox     _i64tow
#define _ui64tox    _ui64tow

/* String functions */
#define _xcscat     wcscat
#define _xcschr     wcschr
#define _xcscpy     wcscpy
#define _xcscspn    wcscspn
#define _xcslen     wcslen
#define _xcsncat    wcsncat
#define _xcsncpy    wcsncpy
#define _xcspbrk    wcspbrk
#define _xcsrchr    wcsrchr
#define _xcsspn     wcsspn
#define _xcsstr     wcsstr
#define _xcstok     wcstok
#define _xcserror   _wcserror
#define __xcserror  __wcserror

#define _xcsdup     _wcsdup
#define _xcsnset    _wcsnset
#define _xcsrev     _wcsrev
#define _xcsset     _wcsset

#define _xcscmp     wcscmp
#define _xcsicmp    _wcsicmp
#define _xcsnccmp   wcsncmp
#define _xcsncmp    wcsncmp
#define _xcsncicmp  _wcsnicmp
#define _xcsnicmp   _wcsnicmp

#define _xcscoll    wcscoll
#define _xcsicoll   _wcsicoll
#define _xcsnccoll  _wcsncoll
#define _xcsncoll   _wcsncoll
#define _xcsncicoll _wcsnicoll
#define _xcsnicoll  _wcsnicoll

/* Execute functions */
#define _xexecl     _wexecl
#define _xexecle    _wexecle
#define _xexeclp    _wexeclp
#define _xexeclpe   _wexeclpe
#define _xexecv     _wexecv
#define _xexecve    _wexecve
#define _xexecvp    _wexecvp
#define _xexecvpe   _wexecvpe

#define _xspawnl    _wspawnl
#define _xspawnle   _wspawnle
#define _xspawnlp   _wspawnlp
#define _xspawnlpe  _wspawnlpe
#define _xspawnv    _wspawnv
#define _xspawnve   _wspawnve
#define _xspawnvp   _wspawnvp
#define _xspawnvpe  _wspawnvpe

#define _xsystem    _wsystem

/* Time functions */
#define _xasctime   _wasctime
#define _xctime     _wctime
#define _xctime64   _wctime64
#define _xstrdate   _wstrdate
#define _xstrtime   _wstrtime
#define _xutime     _wutime
#define _xutime64   _wutime64
#define _xcsftime   wcsftime

/* Directory functions */
#define _xchdir     _wchdir
#define _xgetcwd    _wgetcwd
#define _xgetdcwd   _wgetdcwd
#define _xmkdir     _wmkdir
#define _xrmdir     _wrmdir

/* Environment/Path functions */
#define _xfullpath  _wfullpath
#define _xgetenv    _wgetenv
#define _xmakepath  _wmakepath
#define _xpgmptr    _wpgmptr
#define _xputenv    _wputenv
#define _xsearchenv _wsearchenv
#define _xsplitpath _wsplitpath

/* Stdio functions */
#define _xfdopen    _wfdopen
#define _xfsopen    _wfsopen
#define _xfopen     _wfopen
#define _xfreopen   _wfreopen
#define _xperror    _wperror
#define _xpopen     _wpopen
#define _xtempnam   _wtempnam
#define _xtmpnam    _wtmpnam

/* Io functions */
#define _xaccess    _waccess
#define _xchmod     _wchmod
#define _xcreat     _wcreat
#define _xfindfirst _wfindfirst
#define _xfindfirst64   _wfindfirst64
#define _xfindfirsti64  _wfindfirsti64
#define _xfindnext  _wfindnext
#define _xfindnext64    _wfindnext64
#define _xfindnexti64   _wfindnexti64
#define _xmktemp    _wmktemp
#define _xopen      _wopen
#define _xremove    _wremove
#define _xrename    _wrename
#define _xsopen     _wsopen
#define _xunlink    _wunlink

#define _xfinddata_t    _wfinddata_t
#define _xfinddata64_t  __wfinddata64_t
#define _xfinddatai64_t _wfinddatai64_t

/* Stat functions */
#define _xstat      _wstat
#define _xstat64    _wstat64
#define _xstati64   _wstati64

/* Setlocale functions */
#define _xsetlocale _wsetlocale

/* Redundant "logical-character" mappings */
#define _xcsclen    wcslen
#define _xcsnccat   wcsncat
#define _xcsnccpy   wcsncpy
#define _xcsncset   _wcsnset

/* MBCS-specific mappings */
#define _xcsdec(_cpc1, _cpc2) ((_cpc1)>=(_cpc2) ? NULL : (_cpc2)-1)
#define _xcsinc(_pc)    ((_pc)+1)
#define _xcsnbcnt(_cpc, _sz) ((wcslen(_cpc)>_sz) ? _sz : wcslen(_cpc))
#define _xcsnccnt(_cpc, _sz) ((wcslen(_cpc)>_sz) ? _sz : wcslen(_cpc))
#define _xcsnextc(_cpc) ((unsigned int) *(_cpc))
#define _xcsninc(_pc, _sz) (((_pc)+(_sz)))
#define _xcsspnp(_cpc1, _cpc2) ((*((_cpc1)+wcsspn(_cpc1,_cpc2))) ? ((_cpc1)+wcsspn(_cpc1,_cpc2)) : NULL)

#define _xcslwr     _wcslwr
#define _xcsupr     _wcsupr
#define _xcsxfrm    wcsxfrm

#if _MSC_FULL_VER >= 1400
#define _xcslwr_s   _wcslwr_s
#define _xcsupr_s   _wcsupr_s
#else
#define _xcslwr_s(s,l)   _wcslwr((s))
#define _xcsupr_s(s,l)   _wcsupr((s))
#endif

#define _xclen(_pc) (1)
#define _xccpy(_pc1,_cpc2) ((*(_pc1) = *(_cpc2)))
#define _xccmp(_cpc1,_cpc2) ((*(_cpc1))-(*(_cpc2)))

/* ctype functions */
#define _isxalnum   iswalnum
#define _isxalpha   iswalpha
#define _isxascii   iswascii
#define _isxcntrl   iswcntrl
#define _isxdigit   iswdigit
#define _isxgraph   iswgraph
#define _isxlower   iswlower
#define _isxprint   iswprint
#define _isxpunct   iswpunct
#define _isxspace   iswspace
#define _isxupper   iswupper
#define _isxxdigit  iswxdigit

#define _xotupper   towupper
#define _xotlower   towlower

#define _isxlegal(_c)		(1)
#define _isxlead(_c)		(0)
#define _isxleadbyte(_c)    (0)

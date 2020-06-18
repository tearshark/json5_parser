#define _XEOF       EOF
#define __X(x)      x
#define X_UNICODE	0

/* Program */
#define _xmain      main
#define _xWinMain   WinMain
#define _xenviron  _environ
#define __xargv     __argv

/* Formatted i/o */
#define _xprintf    printf
#define _xcprintf   _cprintf
#define _fxprintf   fprintf
#define _sxprintf   sprintf
#define _scxprintf  _scprintf
#define _snxprintf  snprintf
#define _snxprintf_s  _snprintf_s
#define _vxprintf   vprintf
#define _vfxprintf  vfprintf
#define _vsxprintf  vsnprintf
#define _vscxprintf _vscprintf
#if _MSC_FULL_VER >= 1400
#define _vsnxprintf _vsnprintf
#else
#define _vsnxprintf vsnprintf
#endif
#define _xscanf     scanf
#define _xcscanf    _cscanf
#define _fxscanf    fscanf
#define _sxscanf    sscanf
#define _snxscanf   _snscanf

#if _MSC_FULL_VER >= 1400
#define _sxscanf_s  sscanf_s
#define _vsnxprintf_s _vsnprintf_s
#else
#define _sxscanf_s  sscanf
#define _vsnxprintf_s(_DstBuf,_DstSize,_MaxCount,_Format,_ArgList) _vsnprintf((_DstBuf),(_MaxCount),(_Format),(_ArgList))
#endif

/* Unformatted i/o */
#define _fgetxc     fgetc
#define _fgetxchar  _fgetchar
#define _fgetxs     fgets
#define _fputxc     fputc
#define _fputxchar  _fputchar
#define _fputxs     fputs
#define _cputxs     _cputs
#define _cgetxs     _cgets
#define _getxc      getc
#define _getxch     _getch
#define _getxche    _getche
#define _getxchar   getchar
#define _getxs      gets
#define _putxc      putc
#define _putxchar   putchar
#define _putxch     _putch
#define _putxs      puts
#define _ungetxc    ungetc
#define _ungetxch   _ungetch

/* String conversion functions */
#define _xcstod     strtod
#define _xcstol     strtol
#define _xcstoll	strtoll
#define _xcstoul    strtoul
#define _xcstoull	strtoull
#define _xstof      atof
#define _xstol      atol
#define _xstoi      atoi
#define _xstoi64    _atoi64

#define _itox       _itoa
#define _ltox       _ltoa
#define _ultox      _ultoa
#define _xtoi       atoi
#define _xtol       atol

#define _xtoi64     _atoi64
#define _xcstoi64   _strtoi64
#define _xcstoui64  _strtoui64
#define _i64tox     _i64toa
#define _ui64tox    _ui64toa

/* String functions */
#define _xcscat     strcat
#define _xcschr     strchr
#define _xcscpy     strcpy
#define _xcscspn    strcspn
#define _xcslen     strlen
#define _xcsncat    strncat
#define _xcsncpy    strncpy
#define _xcspbrk    strpbrk
#define _xcsrchr    strrchr
#define _xcsspn     strspn
#define _xcsstr     strstr
#define _xcstok     strtok
#define _xcserror   strerror
#define __xcserror  _strerror

#define _xcsdup     _strdup
#define _xcsnset    _strnset
#if defined(WIN32)
#define _xcsrev     _strrev
#else
#define _xcsrev     strrev
#endif
#define _xcsset     _strset

#define _xcscmp     strcmp
#if defined(WIN32)
#define _xcsicmp    _stricmp
#else
#define _xcsicmp    stricmp
#endif
#define _xcsnccmp   strncmp
#define _xcsncmp    strncmp
#define _xcsncicmp  strnicmp
#define _xcsnicmp   strnicmp

#define _xcscoll    strcoll
#if defined(WIN32)
#define _xcsicoll   _stricoll
#else
#define _xcsicoll   stricmp
#endif
#define _xcsnccoll  _strncoll
#define _xcsncoll   _strncoll
#define _xcsncicoll _strnicoll
#define _xcsnicoll  _strnicoll

/* Execute functions */
#define _xexecl     _execl
#define _xexecle    _execle
#define _xexeclp    _execlp
#define _xexeclpe   _execlpe
#define _xexecv     _execv
#define _xexecve    _execve
#define _xexecvp    _execvp
#define _xexecvpe   _execvpe

#define _xspawnl    _spawnl
#define _xspawnle   _spawnle
#define _xspawnlp   _spawnlp
#define _xspawnlpe  _spawnlpe
#define _xspawnv    _spawnv
#define _xspawnve   _spawnve
#define _xspawnvp   _spawnvp
#define _xspawnvpe  _spawnvpe

#define _xsystem    system

/* Time functions */
#define _xasctime   asctime
#define _xctime     ctime
#define _xctime64   _ctime64
#define _xstrdate   _strdate
#define _xstrtime   _strtime
#define _xutime     _utime
#define _xutime64   _utime64
#define _xcsftime   strftime

/* Directory functions */
#define _xchdir     _chdir
#define _xgetcwd    _getcwd
#define _xgetdcwd   _getdcwd
#define _xmkdir     _mkdir
#define _xrmdir     _rmdir

/* Environment/Path functions */
#define _xfullpath  _fullpath
#define _xgetenv    getenv
#define _xmakepath  _makepath
#define _xpgmptr    _pgmptr
#define _xputenv    _putenv
#define _xsearchenv _searchenv
#define _xsplitpath _splitpath

/* Stdio functions */
#define _xfdopen    _fdopen
#define _xfsopen    _fsopen
#define _xfopen     fopen
#define _xfreopen   freopen
#define _xperror    perror
#define _xpopen     _popen
#define _xtempnam   _tempnam
#define _xtmpnam    tmpnam

/* Io functions */
#define _xaccess    _access
#define _xchmod     _chmod
#define _xcreat     _creat
#define _xfindfirst _findfirst
#define _xfindfirst64   _findfirst64
#define _xfindfirsti64  _findfirsti64
#define _xfindnext  _findnext
#define _xfindnext64    _findnext64
#define _xfindnexti64   _findnexti64
#define _xmktemp    _mktemp
#define _xopen      _open
#define _xremove    remove
#define _xrename    rename
#define _xsopen     _sopen
#define _xunlink    _unlink

#define _xfinddata_t    _finddata_t
#define _xfinddata64_t  __finddata64_t
#define _xfinddatai64_t _finddatai64_t

/* Stat functions */
#define _xstat      _stat
#define _xstat64    _stat64
#define _xstati64   _stati64

/* Setlocale functions */
#define _xsetlocale setlocale

/* Redundant "logical-character" mappings */
#define _xcsclen    strlen
#define _xcsnccat   strncat
#define _xcsnccpy   strncpy
#define _xcsncset   strnset

/* MBCS-specific mappings */
#define _xcsdec(_cpc1, _cpc2) ((_cpc1)>=(_cpc2) ? NULL : (_cpc2)-1)
#define _xcsinc(_pc)    ((_pc)+1)
#define _xcsnbcnt(_cpc, _sz) ((strlen(_cpc)>_sz) ? _sz : strlen(_cpc))
#define _xcsnccnt(_cpc, _sz) ((strlen(_cpc)>_sz) ? _sz : strlen(_cpc))
#define _xcsnextc(_cpc) ((unsigned int) *(const unsigned char *)(_cpc))
#define _xcsninc(_pc, _sz) (((_pc)+(_sz)))
#define _xcsspnp(_cpc1, _cpc2) ((*((_cpc1)+strspn(_cpc1,_cpc2))) ? ((_cpc1)+strspn(_cpc1,_cpc2)) : NULL)

#define _xcslwr     _strlwr
#define _xcsupr     _strupr
#define _xcsxfrm    strxfrm

#if defined(WIN32)
#define _xcslwr_s   _strlwr_s
#define _xcsupr_s   _strupr_s
//#define _xcslwr_s(s,l)   _strlwr(s)
//#define _xcsupr_s(s,l)   _strupr(s)
#else
#define _xcslwr_s(s,l)   strlwr(s)
#define _xcsupr_s(s,l)   strupr(s)
#endif

#define _xclen(_pc) (1)
#define _xccpy(_pc1,_cpc2) (*(_pc1) = *(_cpc2))
#define _xccmp(_cpc1,_cpc2) (((unsigned char)*(_cpc1))-((unsigned char)*(_cpc2)))

/* ctype functions */
#define _isxalnum   isalnum
#define _isxalpha   isalpha
#define _isxascii   isascii
#define _isxcntrl   iscntrl
#define _isxdigit   isdigit
#define _isxgraph   isgraph
#define _isxlower   islower
#define _isxprint   isprint
#define _isxpunct   ispunct
#define _isxspace   isspace
#define _isxupper   isupper
#define _isxxdigit  isxdigit

#define _xotupper   toupper
#define _xotlower   tolower

#define _isxlegal   (1)
#define _isxlead	_ismbblead
#define _isxleadbyte(c)    IsDBCSLeadByte((c))

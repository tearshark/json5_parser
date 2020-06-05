#undef  _XEOF       
#undef  __X
#undef  X_UNICODE

/* Program */
#undef  _xmain      
#undef  _xWinMain   
#undef  _xenviron
#undef  __xargv     

/* Formatted i/o */
#undef  _xprintf    
#undef  _xcprintf   
#undef  _fxprintf   
#undef  _sxprintf   
#undef  _scxprintf  
#undef  _snxprintf  
#undef  _snxprintf_s
#undef  _vxprintf   
#undef  _vfxprintf  
#undef  _vsxprintf  
#undef  _vscxprintf 
#undef  _vsnxprintf 
#undef  _xscanf     
#undef  _xcscanf    
#undef  _fxscanf    
#undef  _sxscanf    
#undef  _snxscanf   

#undef  _sxscanf_s    
#undef  _vsnxprintf_s    

/* Unformatted i/o */
#undef  _fgetxc    
#undef  _fgetxchar 
#undef  _fgetxs    
#undef  _fputxc    
#undef  _fputxchar 
#undef  _fputxs    
#undef  _cputxs    
#undef  _cgetxs    
#undef  _getxc     
#undef  _getxch    
#undef  _getxche   
#undef  _getxchar  
#undef  _getxs     
#undef  _putxc     
#undef  _putxchar  
#undef  _putxch    
#undef  _putxs     
#undef  _ungetxc   
#undef  _ungetxch  

/* String conversion functions */
#undef  _xcstod     
#undef  _xcstol     
#undef  _xcstoll
#undef  _xcstoul    
#undef  _xcstoull

#undef  _xstof      
#undef  _xstol      
#undef  _xstoi      
#undef  _xstoi64    

#undef  _itox       
#undef  _ltox       
#undef  _ultox      
#undef  _xtoi       
#undef  _xtol       

#undef  _xtoi64     
#undef  _xcstoi64   
#undef  _xcstoui64  
#undef  _i64tox     
#undef  _ui64tox    

/* String functions */
#undef  _xcscat     
#undef  _xcschr     
#undef  _xcscpy     
#undef  _xcscspn    
#undef  _xcslen     
#undef  _xcsncat    
#undef  _xcsncpy    
#undef  _xcspbrk    
#undef  _xcsrchr    
#undef  _xcsspn     
#undef  _xcsstr     
#undef  _xcstok     
#undef  _xcserror   
#undef  __xcserror  

#undef  _xcsdup     
#undef  _xcsnset    
#undef  _xcsrev     
#undef  _xcsset     

#undef  _xcscmp     
#undef  _xcsicmp    
#undef  _xcsnccmp   
#undef  _xcsncmp    
#undef  _xcsncicmp  
#undef  _xcsnicmp   

#undef  _xcscoll    
#undef  _xcsicoll   
#undef  _xcsnccoll  
#undef  _xcsncoll   
#undef  _xcsncicoll 
#undef  _xcsnicoll  

/* Execute functions */
#undef  _xexecl     
#undef  _xexecle    
#undef  _xexeclp    
#undef  _xexeclpe   
#undef  _xexecv     
#undef  _xexecve    
#undef  _xexecvp    
#undef  _xexecvpe   

#undef  _xspawnl    
#undef  _xspawnle   
#undef  _xspawnlp   
#undef  _xspawnlpe  
#undef  _xspawnv    
#undef  _xspawnve   
#undef  _xspawnvp   
#undef  _xspawnvpe  

#undef  _xsystem    

/* Time functions */
#undef  _xasctime   
#undef  _xctime     
#undef  _xctime64   
#undef  _xstrdate   
#undef  _xstrtime   
#undef  _xutime     
#undef  _xutime64   
#undef  _xcsftime   

/* Directory functions */
#undef  _xchdir     
#undef  _xgetcwd    
#undef  _xgetdcwd   
#undef  _xmkdir     
#undef  _xrmdir     

/* Environment/Path functions */
#undef  _xfullpath  
#undef  _xgetenv    
#undef  _xmakepath  
#undef  _xpgmptr    
#undef  _xputenv    
#undef  _xsearchenv 
#undef  _xsplitpath 

/* Stdio functions */
#undef  _xfdopen   
#undef  _xfsopen   
#undef  _xfopen    
#undef  _xfreopen  
#undef  _xperror   
#undef  _xpopen    
#undef  _xtempnam  
#undef  _xtmpnam   

/* Io functions */
#undef  _xaccess    
#undef  _xchmod     
#undef  _xcreat     
#undef  _xfindfirst 
#undef  _xfindfirst64
#undef  _xfindfirsti64
#undef  _xfindnext 
#undef  _xfindnext64
#undef  _xfindnexti64
#undef  _xmktemp    
#undef  _xopen      
#undef  _xremove    
#undef  _xrename    
#undef  _xsopen     
#undef  _xunlink    

#undef  _xfinddata_t    
#undef  _xfinddata64_t  
#undef  _xfinddatai64_t 

/* Stat functions */
#undef  _xstat      
#undef  _xstat64    
#undef  _xstati64   

/* Setlocale functions */
#undef  _xsetlocale 

/* Redundant "logical-character" mappings */
#undef  _xcsclen    
#undef  _xcsnccat   
#undef  _xcsnccpy   
#undef  _xcsncset   

/* MBCS-specific mappings */
#undef  _xcsdec     
#undef  _xcsinc     
#undef  _xcsnbcnt   
#undef  _xcsnccnt   
#undef  _xcsnextc   
#undef  _xcsninc    
#undef  _xcsspnp    

#undef  _xcslwr     
#undef  _xcsupr     
#undef  _xcsxfrm    

#undef _xcslwr_s
#undef _xcsupr_s

#undef  _xclen      
#undef  _xccpy      
#undef  _xccmp

/* ctype functions */
#undef  _isxalnum   
#undef  _isxalpha   
#undef  _isxascii   
#undef  _isxcntrl   
#undef  _isxdigit   
#undef  _isxgraph   
#undef  _isxlower   
#undef  _isxprint   
#undef  _isxpunct   
#undef  _isxspace   
#undef  _isxupper   
#undef  _isxxdigit  

#undef  _xotupper   
#undef  _xotlower   

#undef  _isxlegal   
#undef  _isxlead	
#undef  _isxleadbyte   

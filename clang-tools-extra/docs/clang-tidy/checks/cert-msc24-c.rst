.. title:: clang-tidy - cert-msc24-c

cert-msc24-c
============

Checks for deprecated and obsolescent functions listed in
CERT C Coding Standard Recommendation MSC24-C (`MSC24-C. Do not use deprecated or obsolescent functions
<https://wiki.sei.cmu.edu/confluence/display/c/MSC24-C.+Do+not+use+deprecated+or+obsolescent+functions>`_.).
For the listed functions, an alternative, more secure replacement is suggested, if available.
The checker heavily relies on the functions from annex K (Bounds-checking interfaces) of C11.

For the following functions, replacements are suggested from annex K: `asctime`, 
`ctime`, `fopen`, `freopen`, `bsearch`, `fprintf`, `fscanf`, `fwprintf`, `fwscanf`, 
`getenv`, `gmtime`, `localtime`, `mbsrtowcs`, `mbstowcs`, `memcpy`, `memmove`, `printf`, 
`qsort`, `snprintf`, `sprintf`,  `sscanf`, `strcat`, `strcpy`, `strerror`, 
`strncat`, `strncpy`, `strtok`, `swprintf`, `swscanf`, `vfprintf`, `vfscanf`, `vfwprintf`, 
`vfwscanf`, `vprintf`, `vscanf`, `vsnprintf`, `vsprintf`, `vsscanf`, `vswprintf`, 
`vswscanf`, `vwprintf`, `vwscanf`, `wcrtomb`, `wcscat`, `wcscpy`, `wcsncat`, `wcsncpy`, 
`wcsrtombs`, `wcstok`, `wcstombs`, `wctomb`, `wmemcpy`, `wmemmove`, `wprintf`, `wscanf`. 
If annex K is not available, the checker ignores these functions.

The availability of annex K is checked based on the following macros.
 - `__STDC_LIB_EXT1__`: feature macro, which indicates the presence of
   annex K (Bounds-checking interfaces) in the library implementation
 - `__STDC_WANT_LIB_EXT1__`: user defined macro, which indicates if the user wants the functions from
   annex K to be defined.

Both macros have to be defined to suggest replacement functions from annex K. __STDC_LIB_EXT1__ is
defined by the library implementation, and __STDC_WANT_LIB_EXT1__ must be define to "1" by the user 
before including any system headers.

Deprecated function `gets` is checked, and a warning is issued if used.

The following functions are also checked, and alternative replacement functions are suggested:
 - `rewind`, suggested replacement: `fseek`
 - `setbuf`, suggested replacement: `setvbuf`

The following functions are covered in the check `cert-err34-c <cert-err34-c.html>`_,
so this checker **ignores** them: `atof`, `atoi`, `atol`, `atoll`.

Examples:

.. code-block:: c++
  
  //__STDC_LIB_EXT1__ is defined by the library implementation
  #define __STDC_WANT_LIB_EXT1__ 1

  #include <string.h> // defines the functions from annex K
  #include <stdio.h>
  
  enum { BUFSIZE = 32 };

  void fWarning(const char *msg) { 
    static const char prefix[] = "Error: ";
    static const char suffix[] = "\n";
    char buf[BUFSIZE] = {0}; 
    
    strcpy(buf, prefix); // warning: function 'strcpy' is obsolescent; 'strcpy_s' should be used instead.
    strcat(buf, msg); // warning: function 'strcat' is obsolescent; 'strcat_s' should be used instead.
    strcat(buf, suffix); // warning: function 'strcat' is obsolescent; 'strcat_s' should be used instead.
    if (fputs(buf, stderr) < 0) {
      // error handling
      return;
    }
  }

  void fUsingSafeFunctions(const char *msg) { 
    static const char prefix[] = "Error: ";
    static const char suffix[] = "\n";
    char buf[BUFSIZE] = {0}; 
    
    if (strcpy_s(buf, prefix) != 0) {
      // error handling
      return;
    }

    if (strcat_s(buf, msg) != 0) {
      // error handling
      return;
    }
    
    if (strcat_s(buf, suffix) != 0) {
      // error handling
      return;
    }
    
    if (fputs(buf, stderr) < 0) {
      // error handling
      return;
    }
  }

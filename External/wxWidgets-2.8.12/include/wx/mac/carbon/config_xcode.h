///////////////////////////////////////////////////////////////////////////////
// Name:        config_xcode.pch
// Purpose:     configurations for xcode builds
// Author:      Stefan Csomor
// Modified by:
// Created:     29.04.04
// RCS-ID:      $Id: config_xcode.h 66958 2011-02-18 23:42:58Z JS $
// Copyright:   (c) wxWidgets team
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// from config.log confdefs

#define HAVE_SSIZE_T 1
#define STDC_HEADERS 1
#if __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#endif
#define wxUSE_UNIX 1
#define __UNIX__ 1
#define __BSD__ 1
#define __DARWIN__ 1
#define wx_USE_NANOX 0
#define TARGET_CARBON 1

#define HAVE_BOOL 1
#define HAVE_EXPLICIT 1
#define HAVE_CONST_CAST
#define HAVE_REINTERPRET_CAST
#define HAVE_STATIC_CAST
#define HAVE_VA_COPY 1
#define HAVE_VARIADIC_MACROS 1
#define CONST_COMPATIBILITY 0
#define WX_TIMEZONE timezone
#define WX_SOCKLEN_T socklen_t
#define SOCKOPTLEN_T socklen_t
#define WX_STATFS_T struct statfs
#define wxTYPE_SA_HANDLER int
#define WX_GMTOFF_IN_TM 1
#define HAVE_PW_GECOS 1
#define HAVE_DLOPEN 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_FSYNC 1
#define HAVE_LARGEFILE_SUPPORT 1
#define HAVE_LOCALTIME 1
#define HAVE_SCHED_YIELD 1
#define HAVE_PTHREAD_MUTEXATTR_T 1
#define HAVE_PTHREAD_MUTEXATTR_SETTYPE_DECL 1
#define HAVE_PTHREAD_CANCEL 1
#define HAVE_PTHREAD_ATTR_SETSTACKSIZE 1
#define HAVE_SIGACTION 1
#define HAVE_SNPRINTF 1
#define HAVE_SNPRINTF_DECL 1
#define HAVE_UNIX98_PRINTF 1
#define HAVE_STATFS 1
#define HAVE_STATFS_DECL 1
#define HAVE_STRPTIME 1
#define HAVE_STRPTIME_DECL 1
#define HAVE_THREAD_PRIORITY_FUNCTIONS 1
#define HAVE_TIMEGM 1
#define HAVE_VSNPRINTF 1
#define HAVE_VSNPRINTF_DECL 1
#define HAVE_USLEEP 1
#define HAVE_WCSLEN 1
#define SIZEOF_CHAR 1
#define SIZEOF_WCHAR_T 4
#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#ifdef __LP64__
#define SIZEOF_VOID_P 8
#define SIZEOF_LONG 8
#define SIZEOF_SIZE_T 8
#else
#define SIZEOF_VOID_P 4
#define SIZEOF_LONG 4
#define SIZEOF_SIZE_T 4
#endif

#define SIZEOF_LONG_LONG 8
#define wxSIZE_T_IS_ULONG 1
#define HAVE_FCNTL 1
#define HAVE_GETHOSTBYNAME 1
#define HAVE_GETSERVBYNAME 1
#define HAVE_GMTIME_R 1
#define HAVE_INET_ADDR 1
#define HAVE_INET_ATON 1
#define HAVE_LOCALTIME_R 1
#define HAVE_MKSTEMP 1
#define HAVE_PUTENV 1
#define HAVE_READDIR_R 1
#define HAVE_STRTOK_R 1
#define HAVE_UNAME 1
#define HAVE_USLEEP 1
#define HAVE_X11_XKBLIB_H 1
#define HAVE_FNMATCH 1
#define HAVE_FNMATCH_H 1
#define HAVE_IOSTREAM 1
#define HAVE_SCHED_H 1
#define HAVE_UNISTD_H 1
#define HAVE_WCHAR_H 1
#define HAVE_ICONV 1
#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif
#define HAVE_LANGINFO_H 1
#define HAVE_WCSRTOMBS 1
#define HAVE_FPUTWS 1
#define HAVE_STRCASECMP_IN_STRING_H 1
#define HAVE_WPRINTF 1
#define HAVE_SWPRINTF 1
#define HAVE_VSWPRINTF 1
#define HAVE_FSEEKO 1
#define HAVE_SYS_SELECT_H 1

// #define WXWIN_OS_DESCRIPTION "Darwin 7.9.0 Power Macintosh"
// #define PACKAGE_BUGREPORT "wx-dev@lists.wxwidgets.org"
// #define PACKAGE_NAME "wxWidgets"
// #define PACKAGE_STRING "wxWidgets 2.8.12"
// #define PACKAGE_TARNAME "wxwidgets"
// #define PACKAGE_VERSION "2.8.12"

// for regex
#define WX_NO_REGEX_ADVANCED 1

// for jpeg

#define HAVE_STDLIB_H 1

// OBSOLETE ?

#define HAVE_COS 1
#define HAVE_FLOOR 1
#define HAVE_INTTYPES_H 1
#define HAVE_MEMORY_H 1

#define HAVE_REGCOMP 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_X11_XLIB_H 1
#define SOCKLEN_T socklen_t
#define _FILE_OFFSET_BITS 64

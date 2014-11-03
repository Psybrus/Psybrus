/**************************************************************************
 *
 * File:	BcPortability.h
 * Author: 	Neil Richardson
 * Ver/Date:	
 * Description:
 *			Header for determining platform, architecture, and compilers.
 *		
 *		
 *		
 * 
 **************************************************************************/

#ifndef __BCPORTABILITY_H__
#define __BCPORTABILITY_H__


//////////////////////////////////////////////////////////////////////////
// Platform Identification
#define PLATFORM_LINUX			0
#define PLATFORM_WINDOWS		0
#define PLATFORM_IOS			0
#define PLATFORM_OSX			0

// Emscripten (HTML5)
#if defined( EMSCRIPTEN ) || defined( __EMSCRIPTEN__ )
#  undef PLATFORM_HTML5
#  define PLATFORM_HTML5		1

// Linux
#elif defined( linux ) || defined( __linux )
#  undef PLATFORM_LINUX
#  define PLATFORM_LINUX		1
	
// Windows
#elif defined( WINDOWS ) || defined( _WINDOWS )
#  undef PLATFORM_WINDOWS
#  define PLATFORM_WINDOWS		1

// iOS
#elif defined( __APPLE__ ) && ( TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR )
#  undef PLATFORM_IOS
#  define PLATFORM_IOS			1

// Mac OSX
#elif defined( __APPLE__ ) && defined( __MACH__ )
#  undef PLATFORM_OSX
#  define PLATFORM_OSX			1

// Error.
#else
# error "Unknown platform!"

// END.
#endif

//////////////////////////////////////////////////////////////////////////
// Architecture Identification

// X86_64
#if defined( __ia64__ ) || defined( __IA64__ ) || defined( _IA64 ) || defined( _M_IA64 ) || defined( __x86_64__ ) || defined( _M_X64 )
#  define ARCH_X86_64			1
#  define PSY_ENDIAN_LITTLE		1
#  define PSY_ENDIAN_BIG		0

// i386 (and higher)
#elif defined( __i386__ ) || defined( __i486__ ) || defined( __i586__ ) || defined( __i686__ ) || defined( _M_IX86 )
#  define ARCH_X86				1
#  define PSY_ENDIAN_LITTLE		1
#  define PSY_ENDIAN_BIG		0

// PPC
#elif defined( __ppc__ ) || defined( __PPC__ )
#  define ARCH_PPC				1
#  define PSY_ENDIAN_BIG		1
#  define PSY_ENDIAN_LITTLE		0

// PPC64
#elif defined( __ppc64__ ) || defined( __PPC64__ )
#  define ARCH_PPC64			1
#  define PSY_ENDIAN_BIG		1
#  define PSY_ENDIAN_LITTLE		0

// ARM
#elif defined( __arm__ ) || defined( TARGET_OS_IPHONE )
#  define ARCH_ARM				1
#  define PSY_ENDIAN_LITTLE		1
#  define PSY_ENDIAN_LITTLE		0

// THUMB
#elif defined( __thumb__ )
#  define ARCH_THUMB			1
#  define PSY_ENDIAN_LITTLE		1
#  define PSY_ENDIAN_LITTLE		0

 // Emscripten (asm.js)
#elif defined( EMSCRIPTEN )
#  define ARCH_ASMJS			1
#  define PSY_ENDIAN_LITTLE		1
#  define PSY_ENDIAN_BIG		0

// END.
#endif

#if !defined( PSY_ENDIAN_LITTLE ) || !defined( PSY_ENDIAN_BIG )
#error "No endianess defined."
#endif

//////////////////////////////////////////////////////////////////////////
// Compiler Identification

// GCC
#if defined( __GNU__ ) || defined( __GNUG__ )
#  define COMPILER_GCC			1
#else
#  define COMPILER_GCC			0
#endif

// LLVM
#if defined( __llvm__ )
#  define COMPILER_LLVM			1
#else
#  define COMPILER_LLVM			0
#endif

// Codewarrior
#if defined( __MWERKS__ )
#  define COMPILER_CW			1
#else
#  define COMPILER_CW			0
#endif

// MSVC
#if defined( _MSC_VER	)
#  define COMPILER_MSVC			1
#else
#  define COMPILER_MSVC			0
#endif

#endif

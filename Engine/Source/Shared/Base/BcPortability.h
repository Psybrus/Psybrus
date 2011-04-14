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

// Linux
#if defined( linux ) || defined( __linux )
#  define PLATFORM_LINUX		1
	
// Windows
#elif defined( WIN32 ) || defined( _WIN32 )
#  define PLATFORM_WIN32		1

// iOS
#elif defined( __APPLE__ ) && ( TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR )
#  define PLATFORM_IOS			1

// Mac OSX
#elif defined( __APPLE__ ) && defined( __MACH__ )
#  define PLATFORM_OSX			1

// Error.
#else
# error "Unknown platform!"

// END.
#endif

//////////////////////////////////////////////////////////////////////////
// Architecture Identification

// X86_64
#if defined( __ia64__ ) || defined( __IA64__ ) || defined( _IA64 ) || defined( _M_IA64 ) || defined( __x86_64__ ) 
#  define ARCH_X86_64			1
#  define PSY_ENDIAN_LITTLE		1

// i386 (and higher)
#elif defined( __i386__ ) || defined( __i486__ ) || defined( __i586__ ) || defined( __i686__ ) || defined( _M_IX86 )
#  define ARCH_I386				1
#  define PSY_ENDIAN_LITTLE		1

// PPC
#elif defined( __ppc__ ) || defined( __PPC__ )
#  define ARCH_PPC				1
#  define PSY_ENDIAN_BIG		1

// PPC64
#elif defined( __ppc64__ ) || defined( __PPC64__ )
#  define ARCH_PPC64			1
#  define PSY_ENDIAN_BIG		1

// ARM
#elif defined( __arm__ ) || defined( TARGET_OS_IPHONE )
#  define ARCH_ARM				1
#  define PSY_ENDIAN_LITTLE		1

// THUMB
#elif defined( __thumb__ )
#  define ARCH_THUMB			1
#  define PSY_ENDIAN_LITTLE		1


// END.
#endif

//////////////////////////////////////////////////////////////////////////
// Compiler Identification

// GCC
#if defined( __GNU__ )
#  define COMPILER_GCC			1

// LLVM
#elif defined( __llvm__ )
#  define COMPILER_LLVM			1

// Codewarrior
#elif defined( __MWERKS__ )
#  define COMPILER_CW			1

// MSVC
#elif defined( _MSC_VER	)
#  define COMPILER_MSVC			1
#endif

#endif

/**************************************************************************
*
* File:		cTypes.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __BCTYPES_H__
#define __BCTYPES_H__

#include "Base/BcPortability.h"

#include <new>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
// Windows defines
#if PLATFORM_WINDOWS

#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4312 )
#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4127 )
#pragma warning ( disable : 4100 )
#pragma warning ( disable : 4201 )

#if COMPILER_MSVC
typedef unsigned _int64				BcU64;
#else
typedef unsigned long long				BcU64;
#endif

typedef unsigned int				BcU32;
typedef	unsigned short				BcU16;
typedef unsigned char				BcU8;

#if COMPILER_MSVC
typedef signed _int64				BcS64;
#else
typedef signed long long				BcS64;
#endif

typedef signed int					BcS32;
typedef	signed short				BcS16;
typedef signed char					BcS8;

typedef	float						BcF32;
typedef	double						BcF64;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef void*						BcHandle;
typedef size_t						BcSize;

#define BcTrue						BcBool( 1 )
#define BcFalse						BcBool( 0 )

#if COMPILER_MSVC
#  define BcBreakpoint				__debugbreak()
#else
#  define BcBreakpoint				asm( "int $3" )
#endif

#define BcErrorCode					0xffffffff
#define BcLogWrite( t )
#define BcUnusedVar( t )			(void)t

#define BcInline					inline

#if COMPILER_MSVC
#define BcForceInline				__forceinline
#else
#define BcForceInline				inline
#endif

#define BcAlign( decl, v )			decl // TODO: Get rid of vectors in stl containers: __declspec( align( v ) ) decl
#define BcOffsetOf( s, m ) 			(size_t)&(((s *)0)->m)

#define BcPrefetch( a )				_mm_prefetch( reinterpret_cast<char*>a, _MM_HINT_NTA )

#define BcArraySize( a )			( sizeof( a ) / sizeof( a[0] ) )

#ifndef NULL
#define NULL						( 0 )
#endif

// Redefine min and max.
#undef min
#define min min

#undef max
#define max max

#include <deque>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <algorithm>

#endif

/////////////////////////////////////////////
// Linux defines
#if PLATFORM_LINUX

#include <stddef.h>


// TODO: Different architectures.
typedef unsigned long long int		BcU64;
typedef unsigned long int			BcU32;
typedef	unsigned short				BcU16;
typedef unsigned char				BcU8;

typedef signed long long int		BcS64;
typedef signed long int				BcS32;
typedef	signed short				BcS16;
typedef signed char					BcS8;

typedef	float						BcF32;
typedef	double						BcF64;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef void*						BcHandle;
typedef size_t						BcSize;

#define BcTrue						BcBool( 1 )
#define BcFalse						BcBool( 0 )
#define BcBreakpoint				asm( "int $3" )
#define BcErrorCode					0xffffffff
#define BcLogWrite( t )
#define BcUnusedVar( t )			(void)t

#define BcInline					inline
#define BcForceInline				inline

#define BcAlign( decl, v )			decl __attribute__ ((aligned (v)))
#define BcOffsetOf( s, m ) 			(size_t)&(((s*)0)->m)

#define BcPrefetch( a )

#ifndef NULL
#define NULL						( 0 )
#endif

#include <deque>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <algorithm>


#endif

/////////////////////////////////////////////
// OSX/IOS defines
#if PLATFORM_OSX || PLATFORM_IOS

#include <deque>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <algorithm>

#if defined( ARCH_X86 )
typedef unsigned long long			BcU64;
typedef unsigned int				BcU32;
typedef	unsigned short				BcU16;
typedef unsigned char				BcU8;

typedef signed long long			BcS64;
typedef signed int					BcS32;
typedef	signed short				BcS16;
typedef signed char					BcS8;

typedef	float						BcF32;
typedef	double						BcF64;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef void*						BcHandle;
typedef size_t						BcSize;

#ifdef PSY_DEBUG
#define BcBreakpoint				asm( "int $3" )
#else
#define BcBreakpoint
#endif

#elif defined( ARCH_X86_64 )
typedef unsigned long				BcU64;
typedef unsigned int				BcU32;
typedef	unsigned short				BcU16;
typedef unsigned char				BcU8;

typedef signed long					BcS64;
typedef signed int					BcS32;
typedef	signed short				BcS16;
typedef signed char					BcS8;

typedef	float						BcF32;
typedef	double						BcF64;
typedef char						BcChar;
typedef bool						BcBool;
typedef void*						BcHandle;
typedef size_t						BcSize;

#ifdef PSY_DEBUG
#define BcBreakpoint				asm( "int $3" )
#else
#define BcBreakpoint
#endif

#elif defined( ARCH_PPC )
typedef unsigned long int			BcU64;
typedef unsigned int				BcU32;
typedef	unsigned short				BcU16;
typedef unsigned char				BcU8;

typedef signed long long int		BcS64;
typedef signed long int				BcS32;
typedef	signed short				BcS16;
typedef signed char					BcS8;

typedef	float						BcF32;
typedef	double						BcF64;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef void*						BcHandle;
typedef size_t						BcSize;

#ifdef PSY_DEBUG
#define BcBreakpoint				asm( "halt" )
#else
#define BcBreakpoint
#endif

#elif defined( ARCH_ARM )
typedef unsigned long long			BcU64;
typedef unsigned int				BcU32;
typedef	unsigned short				BcU16;
typedef unsigned char				BcU8;

typedef signed long long			BcS64;
typedef signed int					BcS32;
typedef	signed short				BcS16;
typedef signed char					BcS8;

typedef	float						BcF32;
typedef	double						BcF64;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef void*						BcHandle;
typedef std::size_t					BcSize;

#ifdef PSY_DEBUG
#define BcBreakpoint				asm( "halt" )
#else
#define BcBreakpoint
#endif

#else
#error No supported architecture specified.
#endif

#define BcTrue						BcBool( 1 )
#define BcFalse						BcBool( 0 )
#define BcErrorCode					0xffffffff
#define BcLogWrite( t )
#define BcUnusedVar( t )			(void)t

#define BcInline					inline
#define BcForceInline				inline

#define BcAlign( decl, v )			decl __attribute__ ((aligned (v)))
#define BcOffsetOf( s, m ) 			(size_t)&(((s*)0)->m)

#define BcPrefetch( a )

#ifndef NULL
#define NULL						( 0 )
#endif

#endif

//////////////////////////////////////////////////////////////////////////
// Placement new wrapper.
template< typename _Ty >
inline _Ty* placement_new( void* pMem )
{
	return new ( pMem ) _Ty();
}

#endif

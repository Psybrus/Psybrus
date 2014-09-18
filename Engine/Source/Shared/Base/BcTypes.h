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
#include <cstdint>

//////////////////////////////////////////////////////////////////////////
// Windows defines
#if PLATFORM_WINDOWS

#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4312 )
#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4127 )
#pragma warning ( disable : 4100 )
#pragma warning ( disable : 4201 )

typedef std::uint64_t				BcU64;
typedef std::uint32_t				BcU32;
typedef	std::uint16_t				BcU16;
typedef std::uint8_t				BcU8;

typedef std::int64_t				BcS64;
typedef std::int32_t				BcS32;
typedef	std::int16_t				BcS16;
typedef std::int8_t					BcS8;

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

// array hack.
#include <array>

namespace std
{
	using std::tr1::array;
};

// cmath will define this. We say no.
#undef DOMAIN


#endif

#endif

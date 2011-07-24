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

#include "BcPortability.h"

#include <new>
#include <memory.h>

//////////////////////////////////////////////////////////////////////////
// Windows defines
#ifdef PLATFORM_WINDOWS

#pragma warning ( disable : 4311 ) 
#pragma warning ( disable : 4312 ) 
#pragma warning ( disable : 4996 )
#pragma warning ( disable : 4127 )
#pragma warning ( disable : 4100 )
#pragma warning ( disable : 4201 )

typedef unsigned _int64				BcU64;
typedef unsigned int				BcU32;
typedef	unsigned short				BcU16;
typedef unsigned char				BcU8;

typedef signed _int64				BcS64;
typedef signed int					BcS32;
typedef	signed short				BcS16;
typedef signed char					BcS8;

typedef	float						BcF32;
typedef	double						BcF64;
typedef float						BcReal;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef int							BcHandle;
typedef size_t						BcSize;

#define BcTrue						BcBool( 1 )
#define BcFalse						BcBool( 0 )
#define BcBreakpoint				__asm { int 3h }
#define BcErrorCode					0xffffffff
#define BcLogWrite( t )
#define BcUnusedVar( t )			(void)t

#define BcInline					inline
#define BcForceInline				__forceinline

#define BcAlign( decl, v )			decl __declspec( align( v ) )
#define BcOffsetOf( s, m ) 			(size_t)&(((s *)0)->m)

#ifndef NULL
#define NULL						( 0 ) 
#endif

// Redefine min and max.
#undef min
#define min min

#undef max
#define max max

// Setup C++0x tr1 workaround.
#include <deque>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <array>

namespace std
{
	using namespace std::tr1;
}

#endif

/////////////////////////////////////////////
// Linux defines
#ifdef PLATFORM_LINUX

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
typedef float						BcReal;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef int							BcHandle;
typedef size_t						BcSize;

#define BcTrue						BcBool( 1 )
#define BcFalse						BcBool( 0 )
#define BcBreakpoint				asm( "int $3" )
#define BcErrorCode					0xffffffff
#define BcLogWrite( t )
#define BcUnusedVar( t )			(void)t

#define BcInline					inline
#define BcForceInline				inline

#define BcAlign( decl, v )			decl
#define BcOffsetOf( s, m ) 			(size_t)&(((s*)0)->m)

#ifndef NULL
#define NULL						( 0 )
#endif

// Setup C++0x.
#include <deque>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <array>

#endif

/////////////////////////////////////////////
// OSX/IOS defines
#if defined( PLATFORM_OSX ) || defined( PLATFORM_IOS )

#include <deque>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <tr1/array>

namespace std
{
	using namespace std::tr1;
}


#if defined( ARCH_I386 )
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
typedef float						BcReal;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef int							BcHandle;
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
typedef float						BcReal;
typedef char						BcChar;
typedef bool						BcBool;
typedef int							BcHandle;
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
typedef float						BcReal;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef int							BcHandle;
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
typedef float						BcReal;
typedef char						BcChar;
typedef BcU32						BcBool;
typedef int							BcHandle;
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

#define BcAlign( decl, v )			__attribute__ ((aligned (v))) decl
#define BcOffsetOf( s, m ) 			(size_t)&(((s*)0)->m)

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

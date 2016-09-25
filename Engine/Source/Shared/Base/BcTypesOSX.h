#pragma once

#include <new>
#include <cstdint>
#include <unistd.h>
#include <signal.h>

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
typedef std::size_t					BcSize;

#define BcTrue						BcBool( 1 )
#define BcFalse						BcBool( 0 )

#define BcBreakpoint				raise( SIGTRAP )

#define BcErrorCode					0xffffffff
#define BcLogWrite( t )
#define BcUnusedVar( t )			(void)t

#define BcInline					inline
#define BcForceInline				inline

#define BcAlign( decl, v )			decl // TODO: Get rid of vectors in stl containers: __declspec( align( v ) ) decl
#define BcOffsetOf( s, m ) 			(size_t)&(((s *)0)->m)

#define BcPrefetch( a )				

#define BcArraySize( a )			( sizeof( a ) / sizeof( a[0] ) )

#ifndef NULL
#define NULL						( 0 )
#endif

// cmath will define this. We say no.
#undef DOMAIN

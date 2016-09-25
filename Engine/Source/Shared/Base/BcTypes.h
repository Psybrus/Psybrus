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

//////////////////////////////////////////////////////////////////////////
// Platform specific.
#if PLATFORM_WINDOWS || PLATFORM_WINPHONE
#  include "Base/BcTypesWindows.h"
#endif

#if PLATFORM_LINUX
#  include "Base/BcTypesLinux.h"
#endif

#if PLATFORM_OSX
#  include "Base/BcTypesOSX.h"
#endif

#if PLATFORM_HTML5
#  include "BcTypesHTML5.h"
#endif

#if PLATFORM_ANDROID
#  include "Base/BcTypesAndroid.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Array size.
#define BcArraySize( a ) ( sizeof( a ) / sizeof( a[0] ) )


//////////////////////////////////////////////////////////////////////////
// Enum class flag operators.
#define DEFINE_ENUM_CLASS_FLAG_OPERATOR( _Type, _Operator ) \
	inline _Type operator _Operator##= ( _Type& A, _Type B ) \
	{ \
		A = (_Type)( (int)A _Operator (int)B ); \
		return A; \
	} \
	inline _Type operator _Operator ( _Type A, _Type B ) \
	{ \
		return (_Type)( (int)A _Operator (int)B ); \
	} 

#define DEFINE_ENUM_CLASS_UNARY_FLAG_OPERATOR( _Type, _Operator ) \
	inline _Type operator _Operator ( _Type A ) \
	{ \
		return (_Type)( _Operator (int)A ); \
	} 

//////////////////////////////////////////////////////////////////////////
// Enum class flag utilities.
template< typename _Enum >
inline bool BcContainsAllFlags( _Enum Value, _Enum Flags )
{
	static_assert( sizeof( _Enum ) <= sizeof( int ), "Enum size too large." );
	return ( (int)Value & (int)Flags ) == (int)Flags;
}

inline bool BcContainsAllFlags( BcU32 Value, BcU32 Flags )
{
	return ( (int)Value & (int)Flags ) == (int)Flags;
}

template< typename _Enum >
inline bool BcContainsAnyFlags( _Enum Value, _Enum Flags )
{
	static_assert( sizeof( _Enum ) <= sizeof( int ), "Enum size too large." );
	return ( (int)Value & (int)Flags ) != 0;
}

inline bool BcContainsAnyFlags( BcU32 Value, BcU32 Flags )
{
	return ( (int)Value & (int)Flags ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// TODO: Move this to a better place...
// Mega super awesome hack.
// Temporary until better demangling is setup.
namespace CompilerUtility
{
	bool Demangle( const char* Name, char* Output, size_t OutputSize );
}

//////////////////////////////////////////////////////////////////////////
// Symbol exporting.
#if PLATFORM_WINDOWS
#define PSY_EXPORT __declspec(dllexport)
#define PSY_IMPORT __declspec(dllimport)
#else
#define PSY_EXPORT
#define PSY_IMPORT
#endif


#endif // include_guard

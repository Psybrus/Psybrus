/**************************************************************************
*
* File:		BcDebug.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __BCDEBUG_H__
#define __BCDEBUG_H__

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// Debugging
extern void BcPrintf( const BcChar* pString, ... );

//////////////////////////////////////////////////////////////////////////
// Defines
#if defined( PSY_DEBUG )

#  define BcAssertMsg( Condition, Message )	\
	if( !( Condition ) ) \
	{ \
		BcPrintf( "ASSERTION FAILED at line %u in file %s:\n\t%s\n", __LINE__, __FILE__, Message ); \
		BcBreakpoint; \
	}

#  define BcAssert( Condition )			BcAssertMsg( Condition, "" )
#  define BcPreCondition( Condition )	BcAssert( Condition )
#  define BcPostCondition( Condition )	BcAssert( Condition )
#  define BcAssertException( Condition, Exception )	\
	if( !( Condition ) ) \
	{ \
		throw Exception; \
	}

#elif defined( PSY_RELEASE )

#  ifdef PLATFORM_WIN32
#  include <windows.h>
#  include <stdio.h>

#    define BcAssertMsg( Condition, Message )	\
	if( !( Condition ) ) \
	{ \
		BcChar Buffer[ 1024 ]; \
		sprintf( Buffer, "ASSERTION FAILED at line %u in file %s:\n\t"#Condition"\t%s\n", __LINE__, __FILE__, Message ); \
		int Ret = ::MessageBoxA( NULL, Buffer, "ASSERTION FAILED", MB_OKCANCEL | MB_ICONEXCLAMATION ); \
		if( Ret == IDCANCEL ) \
			BcBreakpoint; \
	}

#  else
#    define BcAssertMsg( Condition, Message )	\
	if( !( Condition ) ) \
	{ \
		BcPrintf( "ASSERTION FAILED at line %u in file %s:\n\t"#Condition"\t%s\n", __LINE__, __FILE__, Message ); \
		BcBreakpoint; \
	}

#  endif

#  define BcAssert( Condition )			BcAssertMsg( Condition, #Condition )
#  define BcPreCondition( Condition )	BcAssert( Condition )
#  define BcPostCondition( Condition )	BcAssert( Condition )
#  define BcAssertException( Condition, Exception )	\
	if( !( Condition ) ) \
	{ \
		throw Exception; \
	}
	
#else

#  define BcAssertMsg( Condition, Message )
#  define BcAssert( Condition )
#  define BcPreCondition( Condition )
#  define BcPostCondition( Condition )
#  define BcAssertException( Condition, Exception )	\
	if( !( Condition ) ) \
	{ \
		throw Exception; \
	}
	
#endif

#endif


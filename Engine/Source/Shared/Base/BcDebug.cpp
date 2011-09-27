/**************************************************************************
*
* File:		cDebug.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Some simple debug stuff for use everywhere.
*		
*		
*		
* 
**************************************************************************/

#include "BcDebug.h"
#include "BcString.h"
#include "BcMutex.h"
#include "BcScopedLock.h"

#if PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include <iostream>

//////////////////////////////////////////////////////////////////////////
// Statics.
static BcMutex gOutputLock_;

//////////////////////////////////////////////////////////////////////////
// BcPrintf
void BcPrintf( const BcChar* pString, ... )
{
#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
	static BcChar Buffer[ 4096 ];
	BcScopedLock< BcMutex > Lock( gOutputLock_ );

	va_list ArgList;
	va_start( ArgList, pString );
	vsprintf( Buffer, pString, ArgList );
	va_end( ArgList );

#if PLATFORM_WINDOWS
	::OutputDebugStringA( Buffer );
#endif

	printf( "%s", Buffer );
#else
	BcUnusedVar( pString );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcAssertInternal
BcBool BcAssertInternal( const BcChar* pMessage, const BcChar* pFile, int Line )
{
#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
	static BcChar Buffer[ 4096 ];
	BcScopedLock< BcMutex > Lock( gOutputLock_ );
	BcPrintf( "\"%s\"in %s on line %u.\n", pMessage, pFile, Line );
	BcSPrintf( Buffer, "\"%s\"in %s on line %u.\n\nDo you wish to continue?", pMessage, pFile, Line );
	BcMessageBoxReturn MessageReturn = BcMessageBox( "ASSERTION FAILED.", Buffer, bcMBT_YESNO, bcMBI_ERROR );
	return MessageReturn == bcMBR_YES;
#else
	BcUnusedVar( pMessage );
	BcUnusedVar( pFile );
	BcUnusedVar( Line );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcVerifyInternal
BcBool BcVerifyInternal( const BcChar* pMessage, const BcChar* pFile, int Line )
{
#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
	static BcChar Buffer[ 4096 ];
	BcScopedLock< BcMutex > Lock( gOutputLock_ );
	BcPrintf( "\"%s\"in %s on line %u.\n", pMessage, pFile, Line );
	BcSPrintf( Buffer, "\"%s\"in %s on line %u.\n\nIgnore all of these?", pMessage, pFile, Line );
	BcMessageBoxReturn MessageReturn = BcMessageBox( "VERIFICATION FAILED.", Buffer, bcMBT_YESNO, bcMBI_WARNING );
	return MessageReturn == bcMBR_YES;
#else
	BcUnusedVar( pMessage );
	BcUnusedVar( pFile );
	BcUnusedVar( Line );
#endif
}

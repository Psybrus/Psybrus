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

#include "Base/BcDebug.h"
#include "Base/BcString.h"
#include "Base/BcMutex.h"
#include "Base/BcScopedLock.h"

//////////////////////////////////////////////////////////////////////////
// Statics.
static BcMutex gOutputLock_;

//////////////////////////////////////////////////////////////////////////
// BcAssertInternal
BcBool BcAssertInternal( const BcChar* pMessage, const BcChar* pFile, int Line, ... )
{
#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
	static BcChar Buffer[ 4096 ];
	static BcChar MessageBuffer[ 4096 ];
	BcScopedLock< BcMutex > Lock( gOutputLock_ );
	va_list ArgList;
	va_start( ArgList, Line );
#if COMPILER_MSVC
	vsprintf_s( MessageBuffer, pMessage, ArgList );
#else
	vsprintf( MessageBuffer, pMessage, ArgList );
#endif
	va_end( ArgList );
	BcSPrintf( Buffer, "\"%s\" in %s on line %u.\n\nDo you wish to break?", MessageBuffer, pFile, Line );
	BcMessageBoxReturn MessageReturn = BcMessageBox( "ASSERTION FAILED!", Buffer, bcMBT_YESNO, bcMBI_ERROR );

	return MessageReturn == bcMBR_YES;
#else
	BcUnusedVar( pMessage );
	BcUnusedVar( pFile );
	BcUnusedVar( Line );

	return BcFalse;
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcVerifyInternal
BcBool BcVerifyInternal( const BcChar* pMessage, const BcChar* pFile, int Line, ... )
{
#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
	static BcChar Buffer[ 4096 ];
	static BcChar MessageBuffer[ 4096 ];
	BcScopedLock< BcMutex > Lock( gOutputLock_ );
	va_list ArgList;
	va_start( ArgList, Line );
#if COMPILER_MSVC
	vsprintf_s( MessageBuffer, pMessage, ArgList );
#else
	vsprintf( MessageBuffer, pMessage, ArgList );
#endif
	va_end( ArgList );
	BcSPrintf( Buffer, "\"%s\"in %s on line %u.\n\nIgnore next?", MessageBuffer, pFile, Line );
	BcMessageBoxReturn MessageReturn = BcMessageBox( "VERIFICATION FAILED!", Buffer, bcMBT_YESNOCANCEL, bcMBI_WARNING );

	if( MessageReturn == bcMBR_CANCEL )
	{
		BcBreakpoint;
	}

	return MessageReturn == bcMBR_YES;
#else
	BcUnusedVar( pMessage );
	BcUnusedVar( pFile );
	BcUnusedVar( Line );

	return BcFalse;
#endif
}

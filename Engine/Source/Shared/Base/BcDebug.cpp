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
	vsprintf_s( MessageBuffer, pMessage, ArgList );
	va_end( ArgList );
	BcSPrintf( Buffer, "\"%s\" in %s on line %u.\n\nDo you wish to break?", MessageBuffer, pFile, Line );
	BcMessageBoxReturn MessageReturn = BcMessageBox( "ASSERTION FAILED!", Buffer, bcMBT_YESNO, bcMBI_ERROR );

	return MessageReturn == bcMBR_YES;
#else
	BcUnusedVar( pMessage );
	BcUnusedVar( pFile );
	BcUnusedVar( Line );
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
	vsprintf_s( MessageBuffer, pMessage, ArgList );
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
#endif
}

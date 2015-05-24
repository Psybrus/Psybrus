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
#include <mutex>

#include <cstdarg>

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif // PLATFORM_WINDOWS

//////////////////////////////////////////////////////////////////////////
// Statics.
static std::recursive_mutex GlobalLock_;
static BcAssertFunc AssertHandler_;

//////////////////////////////////////////////////////////////////////////
// BcPrintf
void BcPrintf( const BcChar* Text, ... )
{
	static BcChar MessageBuffer[ 4096 ];
	std::lock_guard< std::recursive_mutex > Lock( GlobalLock_ );
	va_list ArgList;
	va_start( ArgList, Text );
#if COMPILER_MSVC
	vsprintf_s( MessageBuffer, Text, ArgList );
#else
	vsprintf( MessageBuffer, Text, ArgList );
#endif

#if PLATFORM_WINDOWS
	::OutputDebugStringA( MessageBuffer );
#endif
	printf( MessageBuffer );
}

//////////////////////////////////////////////////////////////////////////
// BcAssertInternal
BcBool BcAssertInternal( const BcChar* pMessage, const BcChar* pFile, int Line, ... )
{
#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
	static BcChar Buffer[ 4096 ];
	static BcChar MessageBuffer[ 4096 ];
	std::lock_guard< std::recursive_mutex > Lock( GlobalLock_ );
	va_list ArgList;
	va_start( ArgList, Line );
#if COMPILER_MSVC
	vsprintf_s( MessageBuffer, pMessage, ArgList );
#else
	vsprintf( MessageBuffer, pMessage, ArgList );
#endif
	va_end( ArgList );

	// Check for assert handler.
	if( AssertHandler_ )
	{
		return AssertHandler_(  MessageBuffer, pFile, Line );
	}

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
// BcAssertSetHandler
void BcAssertSetHandler( BcAssertFunc Func )
{
	std::lock_guard< std::recursive_mutex > Lock( GlobalLock_ );
	AssertHandler_ = Func;
}

//////////////////////////////////////////////////////////////////////////
// BcAssertGetHandler
BcAssertFunc BcAssertGetHandler()
{
	std::lock_guard< std::recursive_mutex > Lock( GlobalLock_ );
	return AssertHandler_;
}

//////////////////////////////////////////////////////////////////////////
// BcVerifyInternal
BcBool BcVerifyInternal( const BcChar* pMessage, const BcChar* pFile, int Line, ... )
{
#if defined( PSY_DEBUG ) || defined( PSY_RELEASE )
	static BcChar Buffer[ 4096 ];
	static BcChar MessageBuffer[ 4096 ];
	std::lock_guard< std::recursive_mutex > Lock( GlobalLock_ );
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

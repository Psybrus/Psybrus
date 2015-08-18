/**************************************************************************
*
* File:		BcDebugAndroid.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Debug stuff
*		
*
*
* 
**************************************************************************/

#include "Base/BcDebug.h"

#include <malloc.h>

#define SUPPORT_BACKTRACE 0

#if SUPPORT_BACKTRACE
#include <execinfo.h>
#endif // SUPPORT_BACKTRACE

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcBacktraceResult BcBacktrace()
{
	BcBacktraceResult Result;

#if SUPPORT_BACKTRACE
	static const int BacktraceBufferSize = 1024;
	void* BacktraceBuffer[ BacktraceBufferSize ];

	int Size = backtrace( BacktraceBuffer, BacktraceBufferSize );
	auto Symbols = backtrace_symbols( BacktraceBuffer, Size );
	for( BcU32 Idx = 0; Idx < Size; ++Idx )
	{
		BcBacktraceEntry Entry;
		Entry.Address_ = BacktraceBuffer[ Idx ];
		Entry.Symbol_ = Symbols[ Idx ];

		Result.Backtrace_.push_back( Entry );
	}

	// We must fre symbols ourselves.
	free( Symbols );
#endif // SUPPORT_BACKTRACE

	return Result;
}

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcMessageBoxReturn BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, BcMessageBoxType Type, BcMessageBoxIcon Icon )
{
	// Log.
	PSY_LOG( "%s: %s\n", pTitle, pMessage );

	return bcMBR_OK; // bcMBR_CANCEL
}

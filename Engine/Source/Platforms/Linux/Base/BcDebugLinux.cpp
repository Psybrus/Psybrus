/**************************************************************************
*
* File:		BcDebugLinux.cpp
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

#if !PSY_PRODUCTION
#include <execinfo.h>
#endif // !PSY_PRODUCTION

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcBacktraceResult BcBacktrace()
{
	BcBacktraceResult Result;

#if !PSY_PRODUCTION
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
#endif // !PSY_PRODUCTION

	return Result;
}

//////////////////////////////////////////////////////////////////////////
// BcMessageBox
BcMessageBoxReturn BcMessageBox( const BcChar* pTitle, const BcChar* pMessage, BcMessageBoxType Type, BcMessageBoxIcon Icon )
{
	// Log.
	BcPrintf( "%s: %s\n", pTitle, pMessage );

	return bcMBR_OK; // bcMBR_CANCEL
}

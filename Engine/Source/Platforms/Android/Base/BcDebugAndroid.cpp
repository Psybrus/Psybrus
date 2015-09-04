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

#include <unwind.h>
#include <dlfcn.h>

//////////////////////////////////////////////////////////////////////////
// Backtrace utility.
namespace
{
	struct BacktraceState
	{
	    void** Curr_;
	    void** End_;
	};

	static _Unwind_Reason_Code unwindCallback( struct _Unwind_Context* Context, void* Arg )
	{
	    BacktraceState* State = static_cast< BacktraceState* >( Arg );
	    uintptr_t PC = _Unwind_GetIP( Context );
	    if( PC )
	    {
	        if( State->Curr_ == State->End_ )
	        {
	            return _URC_END_OF_STACK;
	        }
	        else
	        {
	            *State->Curr_++ = reinterpret_cast< void* >( PC );
	        }
	    }
	    return _URC_NO_REASON;
	}

	static size_t captureBacktrace( void** Buffer, size_t Max )
	{
	    BacktraceState State = { Buffer, Buffer + Max };
	    _Unwind_Backtrace( unwindCallback, &State );
	    return State.Curr_ - Buffer;
	}
}

//////////////////////////////////////////////////////////////////////////
// BcBacktrace
BcBacktraceResult BcBacktrace()
{
	BcBacktraceResult Result;

#if 0 // Doesn't work, investigate later.
	static const int BacktraceBufferSize = 1024;
	void* BacktraceBuffer[ BacktraceBufferSize ] = { nullptr };
	auto Size = captureBacktrace( BacktraceBuffer, BacktraceBufferSize );
	for( BcU32 Idx = 0; Idx < Size; ++Idx )
	{
		BcBacktraceEntry Entry;
		Entry.Address_ = BacktraceBuffer[ Idx ];
        Dl_info Info;
		if( dladdr( Entry.Address_, &Info ) && Info.dli_sname )
		{
			Entry.Symbol_ = Info.dli_sname;
		}

		Result.Backtrace_.push_back( Entry );
	}
#endif
	
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

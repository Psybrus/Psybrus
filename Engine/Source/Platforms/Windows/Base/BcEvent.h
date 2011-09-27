/**************************************************************************
*
* File:		BcEvent.h
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		A C++ Event Implementation.
*		
*		
* 
**************************************************************************/

#ifndef __BCEVENT_H__
#define __BCEVENT_H__

#include "BcTypes.h"
#include "BcDebug.h"

#include <windows.h>

//////////////////////////////////////////////////////////////////////////
// BcEvent
class BcEvent
{
public:
	BcEvent( const BcChar* Name = NULL );
	~BcEvent();

	BcBool wait( BcU32 TimeoutMS = BcErrorCode );
	void signal();

private:
	HANDLE EventHandle_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcEvent::BcEvent( const BcChar* Name )
{
	EventHandle_ = ::CreateEvent( NULL, false, false, Name );
}

inline BcEvent::~BcEvent()
{
	::CloseHandle( EventHandle_ );
}

inline BcBool BcEvent::wait( BcU32 TimeoutMS )
{
	return ( ::WaitForSingleObject( EventHandle_, ( TimeoutMS == BcErrorCode ) ? INFINITE : TimeoutMS ) == WAIT_OBJECT_0 ); 
}

inline void BcEvent::signal()
{
	::SetEvent( EventHandle_ );
}

#endif

/**************************************************************************
*
* File:		BcEvent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		A C++ Event Implementation.
*		
*		
* 
**************************************************************************/

#include "BcEvent.h"

#include <sys/time.h>
#include <sys/stat.h>

//////////////////////////////////////////////////////////////////////////
// Ctor
BcEvent::BcEvent( const BcChar* Name )
{
	BcUnusedVar( Name );
	sem_init( &SemHandle_, 1, 0 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcEvent::~BcEvent()
{
	sem_destroy( &SemHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// wait
BcBool BcEvent::wait( BcU32 TimeoutMS )
{
	int RetVal = 0;
		
	timespec   TimeSpec;
	timeval    TimeVal;
  
	// If we don't want to time out, just wait normally.
	if( TimeoutMS == BcErrorCode )
	{
		sem_wait( &SemHandle_ );
	}
	else
	{
		// Setup time.
		gettimeofday( &TimeVal, NULL );
		
		// Convert time and add on the timeout.
		TimeSpec.tv_sec  = TimeVal.tv_sec;
		TimeSpec.tv_nsec = ( TimeVal.tv_usec * 1000 );
		TimeSpec.tv_sec +=  ( TimeoutMS / 1000 );
		TimeSpec.tv_nsec += ( TimeoutMS % 1000 ) * 1000000;
			
		// do lock.
#if 0
		RetVal = sem_timedwait( &SemHandle_, &TimeSpec );
#else
		BcBreakpoint;
#endif
	}
	
	// 
	return RetVal == 0 ? BcTrue : BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// signal
void BcEvent::signal()
{
	sem_post( &SemHandle_ );
}

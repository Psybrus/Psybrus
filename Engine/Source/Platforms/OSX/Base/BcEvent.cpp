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
#include "BcDebug.h"

#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

//////////////////////////////////////////////////////////////////////////
// Ctor
BcEvent::BcEvent( const BcChar* Name )
{
	BcUnusedVar( Name );
	pthread_cond_init( &Handle_, NULL );

	int MutexReturn = 0;
	pthread_mutexattr_init( &MutexAttr_ );
	MutexReturn = pthread_mutexattr_settype( &MutexAttr_, PTHREAD_MUTEX_RECURSIVE );
	BcAssert( MutexReturn == 0 );
	pthread_mutex_init( &MutexHandle_, &MutexAttr_ );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcEvent::~BcEvent()
{
	pthread_mutex_destroy( &MutexHandle_ );
	pthread_mutexattr_destroy( &MutexAttr_ );
	pthread_cond_destroy( &Handle_ );
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
		RetVal = pthread_cond_wait( &Handle_, &MutexHandle_ );
	}
	// 
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
		RetVal = pthread_cond_timedwait( &Handle_, &MutexHandle_, &TimeSpec );
	}
	
	// 
	return RetVal != ETIMEDOUT ? BcTrue : BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// signal
void BcEvent::signal()
{
	pthread_cond_signal( &Handle_ );
}

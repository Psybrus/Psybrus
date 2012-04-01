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
	pthread_mutex_lock( &MutexHandle_ );
	if( TimeoutMS == BcErrorCode )
	{
		// If we haven't received a signal, do the wait.
		if( Signal_ == 0 )
		{
			RetVal = pthread_cond_wait( &Handle_, &MutexHandle_ );
		}
	}
	// 
	else
	{
		// If we haven't received the signal, do the wait.
		if( Signal_ == 0 )
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
	}

	// Drop back 1 signal.
	if( Signal_ != 0 )
	{
		--Signal_;
	}

	pthread_mutex_unlock( &MutexHandle_ );

	// 
	return RetVal != ETIMEDOUT ? BcTrue : BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// signal
void BcEvent::signal()
{
	pthread_mutex_lock( &MutexHandle_ );
	++Signal_;
	pthread_cond_signal( &Handle_ );
	pthread_mutex_unlock( &MutexHandle_ );
}

/**************************************************************************
*
* File:		BcMutex.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		A C++ Mutex Implementation.
*		
*		
* 
**************************************************************************/

#include "BcMutex.h"
#include "BcDebug.h"


BcMutex::BcMutex()
{
	int MutexReturn = 0;
	pthread_mutexattr_init( &MutexAttr_ );
	MutexReturn = pthread_mutexattr_settype( &MutexAttr_, PTHREAD_MUTEX_RECURSIVE );
	BcAssert( MutexReturn == 0 );

	pthread_mutex_init( &MutexHandle_, &MutexAttr_ );
}

BcMutex::~BcMutex()
{
	pthread_mutex_destroy( &MutexHandle_ );
	pthread_mutexattr_destroy( &MutexAttr_ );
}

void BcMutex::lock()
{
	pthread_mutex_lock( &MutexHandle_ );
}

void BcMutex::unlock()
{
	pthread_mutex_unlock( &MutexHandle_ );
}
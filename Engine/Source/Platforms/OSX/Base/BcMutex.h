/**************************************************************************
*
* File:		BcMutex.h
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		A C++ Mutex Implementation.
*		
*		
* 
**************************************************************************/

#ifndef __BCMUTEX_H__
#define __BCMUTEX_H__

#include "BcTypes.h"

#include <pthread.h>

//////////////////////////////////////////////////////////////////////////
// BcMutex
class BcMutex
{
public:
	BcMutex();
	~BcMutex();

	void lock();
	void unlock();

private:
	pthread_mutexattr_t MutexAttr_;
	pthread_mutex_t MutexHandle_;
};

#endif

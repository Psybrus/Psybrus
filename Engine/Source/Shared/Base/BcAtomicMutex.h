/**************************************************************************
*
* File:		BcAtomicMutex.h
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		Simple atomic mutex. Used for lightweight non recursive locking
*		where the lock is only gonna be short.
*		
* 
**************************************************************************/

#ifndef __BCATOMICMUTEXS_H__
#define __BCATOMICMUTEXS_H__

#include "Base/BcMisc.h"
#include <atomic>

//////////////////////////////////////////////////////////////////////////
// BcAtomicMutex
class BcAtomicMutex
{
public:
	BcAtomicMutex();
	~BcAtomicMutex();
	
	void lock();
	void unlock();

private:
	std::atomic< BcU8 >		Value_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcAtomicMutex::BcAtomicMutex():
	Value_( 0 )
{
	
}

BcForceInline BcAtomicMutex::~BcAtomicMutex()
{
	
}

BcForceInline void BcAtomicMutex::lock()
{
	while( Value_.exchange( 1 ) == 1 )
	{
		BcYield();
	}
}

BcForceInline void BcAtomicMutex::unlock()
{
	Value_.exchange( 0 );
}

#endif

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

#include <windows.h>

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
	CRITICAL_SECTION CriticalSection_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline BcMutex::BcMutex()
{
	::InitializeCriticalSection( &CriticalSection_ );
}

inline BcMutex::~BcMutex()
{
	::DeleteCriticalSection( &CriticalSection_ );
}

inline void BcMutex::lock()
{
	::EnterCriticalSection( &CriticalSection_ );
}

inline void BcMutex::unlock()
{
	::LeaveCriticalSection( &CriticalSection_ );
}

#endif

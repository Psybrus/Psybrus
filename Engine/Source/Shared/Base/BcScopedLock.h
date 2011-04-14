/**************************************************************************
*
* File:		BcScopedLock.h
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		Scoped locking mechanism for mutexes or similar locks.
*		
*		
* 
**************************************************************************/

#ifndef __BCSCOPEDLOCK_H__
#define __BCSCOPEDLOCK_H__

//////////////////////////////////////////////////////////////////////////
// BcScopedMutex
template< typename _Ty >
class BcScopedLock
{
public:
	BcScopedLock( _Ty& Mutex );
	~BcScopedLock();

private:
	_Ty& Mutex_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty >
BcForceInline BcScopedLock< _Ty >::BcScopedLock( _Ty& Mutex ):
	Mutex_( Mutex )
{
	Mutex_.lock();
}

template< typename _Ty >
BcForceInline BcScopedLock< _Ty >::~BcScopedLock()
{
	Mutex_.unlock();
}

#endif

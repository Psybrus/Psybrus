/**************************************************************************
*
* File:		SysDelegateDispatcher.cpp
* Author:	Neil Richardson 
* Ver/Date:	19/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/SysDelegateDispatcher.h"



//////////////////////////////////////////////////////////////////////////
// Ctor
SysDelegateDispatcher::SysDelegateDispatcher()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SysDelegateDispatcher::~SysDelegateDispatcher()
{
	
}

//////////////////////////////////////////////////////////////////////////
// enqueueDelegateCall
void SysDelegateDispatcher::enqueueDelegateCall( const std::function< void() >& Function )
{
	std::lock_guard< std::mutex > Lock( DelegateCallListLock_ );
	DelegateCallList_.push_back( Function );
}

//////////////////////////////////////////////////////////////////////////
// dispatch
void SysDelegateDispatcher::dispatch()
{
	// Grab list of delegate calls.
	DelegateCallListLock_.lock();
	TDelegateCallList DelegateCallList( DelegateCallList_ );
	DelegateCallList_.clear();
	DelegateCallListLock_.unlock();
	
	// Iterate over em, call, and delete them.
	for( TDelegateCallListIterator It( DelegateCallList.begin() ); It != DelegateCallList.end(); ++It )
	{
		(*It)();
	}
}

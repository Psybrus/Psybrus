/**************************************************************************
*
* File:		SysDelegateDispatcher.h
* Author:	Neil Richardson 
* Ver/Date:	19/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SysDelegateDispatcher_H__
#define __SysDelegateDispatcher_H__

#include "Base/BcTypes.h"
#include <mutex>
#include <functional>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// SysDelegateDispatcher
class SysDelegateDispatcher
{
public:
	SysDelegateDispatcher();
	~SysDelegateDispatcher();
	
	void enqueueDelegateCall( const std::function< void() >& Function );
	void dispatch();
	
private:
	typedef std::vector< std::function< void() > > TDelegateCallList;
	typedef TDelegateCallList::iterator TDelegateCallListIterator;
	
	TDelegateCallList DelegateCallList_;
	std::mutex DelegateCallListLock_;
};

#endif



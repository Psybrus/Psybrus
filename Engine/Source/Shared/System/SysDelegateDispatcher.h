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

#include "BcTypes.h"
#include "BcDelegate.h"
#include "BcMutex.h"

//////////////////////////////////////////////////////////////////////////
// SysDelegateDispatcher
class SysDelegateDispatcher
{
public:
	SysDelegateDispatcher();
	~SysDelegateDispatcher();
	
	void enqueueDelegateCall( BcDelegateCallBase* pDelegateCall );
	void dispatch();
	
private:
	typedef std::vector< BcDelegateCallBase* > TDelegateCallList;
	typedef TDelegateCallList::iterator TDelegateCallListIterator;
	
	TDelegateCallList DelegateCallList_;
	BcMutex DelegateCallListLock_;
};

#endif



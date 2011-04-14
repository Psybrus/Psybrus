/**************************************************************************
*
* File:		BcThread.cpp
* Author: 	Neil Richardson 
* Ver/Date:	0.2 - 31/01/06
*			0.3 - 10/12/06
*			0.4 - 27/02/07
* Description:
*		A C++ Class Thread Implementation.
*		
*		
* 
**************************************************************************/

#include "BcThread.h"

BcThread::BcThread()
{
	IsActive_ = BcFalse;
}
	
BcThread::~BcThread()
{
	if( IsActive_ == BcTrue )
	{
		join();
	}
}

BcU32 BcThread::start()
{
	pthread_create( &ThreadHandle_, NULL, &BcThread::threadMain, this );
	return 0;
}

BcU32 BcThread::join()
{
	void* RetVal;
	pthread_join( ThreadHandle_, &RetVal );
	return 0;
}


BcBool BcThread::isActive()
{
	return IsActive_;
}

//static
void* BcThread::threadMain( void* pParam )
{
	BcThread* pThread = (BcThread*)pParam;
	
	pThread->IsActive_ = BcTrue;
	pThread->execute();
	pThread->IsActive_ = BcFalse;
	
	return 0;
}


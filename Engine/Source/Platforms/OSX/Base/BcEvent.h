/**************************************************************************
*
* File:		BcEvent.h
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		A C++ Event Implementation.
*		
*		
* 
**************************************************************************/

#ifndef __BCEVENT_H__
#define __BCEVENT_H__

#include "BcTypes.h"

#include <pthread.h>

//////////////////////////////////////////////////////////////////////////
// BcEvent
class BcEvent
{
public:
	BcEvent( const BcChar* Name = NULL );
	~BcEvent();

	BcBool wait( BcU32 TimeoutMS );
	void signal();

private:
	pthread_cond_t Handle_;
	pthread_mutexattr_t MutexAttr_;
	pthread_mutex_t MutexHandle_;
};


#endif

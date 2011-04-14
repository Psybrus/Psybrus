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

#include <semaphore.h>

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
	sem_t SemHandle_;

};


#endif

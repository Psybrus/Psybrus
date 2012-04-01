/**************************************************************************
*
* File:		BcTimer.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		High Precision Timer
*		
*
*
* 
**************************************************************************/

#ifndef __BCTIMER_H__
#define __BCTIMER_H__

#include "BcTypes.h"

#include <sys/time.h>

//////////////////////////////////////////////////////////////////////////
// BcTimer
class BcTimer
{
public:
	/**
	*	Mark point of reference.
	*/
	void						mark();

	/**
	*	Read time relative to point of reference.
	*/
	BcReal						time();

private:
	timeval						MarkedTime_;
};

#endif

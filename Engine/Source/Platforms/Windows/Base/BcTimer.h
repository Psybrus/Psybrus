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

#ifndef __BcTIMER_H__
#define __BcTIMER_H__

#include "BcTypes.h"
#include "BcWindows.h"

#define USE_PERF_COUNTER 0 

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
#if USE_PERF_COUNTER
	LARGE_INTEGER				Freq_;
	LARGE_INTEGER				MarkedTime_;
#else
	BcF64						MarkedTime_;
#endif
};

#endif

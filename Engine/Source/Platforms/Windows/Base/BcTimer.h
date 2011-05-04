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
	LARGE_INTEGER				Freq_;
	LARGE_INTEGER				MarkedTime_;
};

#endif

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

#include "Base/BcTypes.h"
#include "Base/BcWindows.h"

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
	BcF64						time();

private:
	LARGE_INTEGER				PerfFreq_;
	LARGE_INTEGER				PerfMarkedTime_;
	BcF64						MarkedTime_;
};

#endif

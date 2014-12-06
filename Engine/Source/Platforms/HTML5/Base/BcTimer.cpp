/**************************************************************************
*
* File:		BcTimer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		High Precision Timer
*		
*
*
* 
**************************************************************************/

#include "Base/BcTimer.h"
#include "Base/BcDebug.h"
#include <emscripten.h>


//////////////////////////////////////////////////////////////////////////
// mark
void BcTimer::mark()
{
	MarkedTime_ = emscripten_get_now();
}

//////////////////////////////////////////////////////////////////////////
// time
BcF64 BcTimer::time()
{
	auto NowTime = emscripten_get_now();
	return BcF64( NowTime - MarkedTime_ );
}

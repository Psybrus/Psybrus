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
	BcF64 CurrTime = emscripten_get_now();
	return BcF64( CurrTime - MarkedTime_ ) / 1000.0;
}

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

#include "BcTimer.h"

//////////////////////////////////////////////////////////////////////////
// mark
void BcTimer::mark()
{
#if USE_PERF_COUNTER
	::QueryPerformanceFrequency( &Freq_ );
	::QueryPerformanceCounter( &MarkedTime_ );
#else
	MarkedTime_ = (BcF64)timeGetTime() / 1000.0;
#endif
}

//////////////////////////////////////////////////////////////////////////
// time
BcReal BcTimer::time()
{
#if USE_PERF_COUNTER
	LARGE_INTEGER Time;
	::QueryPerformanceCounter( &Time );
	LONGLONG DeltaTime = Time.QuadPart - MarkedTime_.QuadPart;
	return BcReal( (double)DeltaTime / (double)Freq_.QuadPart );
#else
	return BcReal( ( (BcF64)timeGetTime() / 1000.0 ) - MarkedTime_ );
#endif
}

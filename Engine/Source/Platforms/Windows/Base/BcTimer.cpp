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

#define USE_PERF_COUNTER 1

//////////////////////////////////////////////////////////////////////////
// mark
void BcTimer::mark()
{
#if USE_PERF_COUNTER
	::QueryPerformanceFrequency( &PerfFreq_ );
	::QueryPerformanceCounter( &PerfMarkedTime_ );
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
	LONGLONG DeltaTime = Time.QuadPart - PerfMarkedTime_.QuadPart;
	return BcReal( (double)DeltaTime / (double)PerfFreq_.QuadPart );
#else
	return BcReal( ( (BcF64)timeGetTime() / 1000.0 ) - MarkedTime_ );
#endif
}

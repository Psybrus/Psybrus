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
	::QueryPerformanceFrequency( &Freq_ );
	::QueryPerformanceCounter( &MarkedTime_ );
}

//////////////////////////////////////////////////////////////////////////
// time
BcReal BcTimer::time()
{
	LARGE_INTEGER Time;

	::QueryPerformanceCounter( &Time );

	LONGLONG DeltaTime = Time.QuadPart - MarkedTime_.QuadPart;

	return BcReal( (double)DeltaTime / (double)Freq_.QuadPart );
}

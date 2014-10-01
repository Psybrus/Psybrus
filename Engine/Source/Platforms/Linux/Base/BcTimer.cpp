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

//////////////////////////////////////////////////////////////////////////
// mark
void BcTimer::mark()
{
	::gettimeofday( &MarkedTime_, NULL );
}

//////////////////////////////////////////////////////////////////////////
// time
BcF64 BcTimer::time()
{
	timeval TimeVal;
	::gettimeofday( &TimeVal, NULL );

	BcF64 MarkedTime = MarkedTime_.tv_sec + ( (BcF64)MarkedTime_.tv_usec / 1000000.0 );
	BcF64 CurrTime = TimeVal.tv_sec + ( (BcF64)TimeVal.tv_usec / 1000000.0 );
	
	return BcF64( CurrTime - MarkedTime );
}

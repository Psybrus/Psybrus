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

//////////////////////////////////////////////////////////////////////////
// mark
void BcTimer::mark()
{
	auto RetVal = ::gettimeofday( &MarkedTime_, NULL );
	BcAssert( RetVal != -1 );
}

//////////////////////////////////////////////////////////////////////////
// time
BcF64 BcTimer::time()
{
	timeval TimeVal;
	auto RetVal = ::gettimeofday( &TimeVal, NULL );
	BcAssert( RetVal != -1 );
	BcF64 MarkedTime = MarkedTime_.tv_sec + ( (BcF64)MarkedTime_.tv_usec / 1000000.0 );
	BcF64 CurrTime = TimeVal.tv_sec + ( (BcF64)TimeVal.tv_usec / 1000000.0 );
	BcAssert( CurrTime >= MarkedTime );
	return BcF64( CurrTime - MarkedTime );
}

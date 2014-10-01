/**************************************************************************
 *
 * File:		BcMisc.cpp
 * Author: 	Neil Richardson 
 * Ver/Date:	
 *			
 * Description:
 *		Misc functions.
 *		
 *		
 * 
 **************************************************************************/

#include "Base/BcMisc.h"
#include "Base/BcDebug.h"

#include <unistd.h>
#include <thread>

//////////////////////////////////////////////////////////////////////////
// BcSleep
void BcSleep( BcF32 Seconds )
{
	::usleep( (useconds_t)( Seconds * 1000000.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// BcYield
void BcYield()
{
	::usleep( 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcGetHardwareThreadCount
BcU32 BcGetHardwareThreadCount()
{
	return std::thread::hardware_concurrency();
}

//////////////////////////////////////////////////////////////////////////
// BcCurrentThreadId
BcThreadId BcCurrentThreadId()
{
	return 0; // TODO/remove?
}

//////////////////////////////////////////////////////////////////////////
// BcSetMainThread
static BcThreadId GMainThreadID = 0;
void BcSetGameThread()
{
	GMainThreadID = BcCurrentThreadId();
}

//////////////////////////////////////////////////////////////////////////
// BcIsGameThread
BcBool BcIsGameThread()
{
	return GMainThreadID == 0 ||						// Game thread not setup yet (static initialisers)
		   GMainThreadID == BcCurrentThreadId();		// Game thread matches.
}

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
#include "Base/BcWindows.h"

//////////////////////////////////////////////////////////////////////////
// BcSleep
void BcSleep( BcF32 Seconds )
{
	::Sleep( (DWORD)( Seconds * 1000.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// BcYield
void BcYield()
{
	::SwitchToThread();
}

//////////////////////////////////////////////////////////////////////////
// BcGetHardwareThreadCount
BcU32 BcGetHardwareThreadCount()
{
	SYSTEM_INFO SysInfo;
	::GetSystemInfo( &SysInfo );
	BcU32 RetVal = (BcU32)SysInfo.dwNumberOfProcessors;
	BcAssert( RetVal >= 1 );
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// BcCurrentThreadId
BcThreadId BcCurrentThreadId()
{
	return (BcThreadId)::GetCurrentThreadId();
}

//////////////////////////////////////////////////////////////////////////
// BcSetMainThread
static BcThreadId GMainThreadID = BcErrorCode;
void BcSetGameThread()
{
	GMainThreadID = BcCurrentThreadId();
}

//////////////////////////////////////////////////////////////////////////
// BcIsGameThread
BcBool BcIsGameThread()
{
	return GMainThreadID == BcErrorCode ||					// Game thread not setup yet (static initialisers)
		   GMainThreadID == BcCurrentThreadId();			// Game thread matches.
}

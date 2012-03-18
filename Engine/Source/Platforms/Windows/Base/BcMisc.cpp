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
void BcSleep( BcReal Seconds )
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
// BcSetMainThread
static DWORD GMainThreadID = BcErrorCode;
void BcSetGameThread()
{
	GMainThreadID = ::GetCurrentThreadId();
}

//////////////////////////////////////////////////////////////////////////
// BcIsGameThread
BcBool BcIsGameThread()
{
	return GMainThreadID == BcErrorCode ||					// Game thread not setup yet (static initialisers)
		   GMainThreadID == ::GetCurrentThreadId();			// Game thread matches.
}

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

#include "BcMisc.h"
#include "BcDebug.h"
#include "BcWindows.h"

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
	::Sleep( 0 );
//	::SwitchToThread();
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

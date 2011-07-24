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
#include "BcWindows.h"

//////////////////////////////////////////////////////////////////////////
// BcSleep
void BcSleep( BcReal Seconds )
{
	::Sleep( (DWORD)( Seconds * 0.001f ) );
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
	return (BcU32)SysInfo.dwNumberOfProcessors;
}

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

#include <sys/sysctl.h>

//////////////////////////////////////////////////////////////////////////
// BcSleep
void BcSleep( BcReal Seconds )
{
	usleep( (useconds_t)( Seconds * 1000000.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// BcYield
void BcYield()
{
	usleep( 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcGetHardwareThreadCount
BcU32 BcGetHardwareThreadCount()
{
	int NumCPU = 1;
	int MIB[4];
	size_t Len = sizeof( NumCPU ); 
	
	MIB[0] = CTL_HW;
	MIB[1] = HW_AVAILCPU;
	
	sysctl( MIB, 2, &NumCPU, &Len, NULL, 0 );
	
	if( NumCPU < 1 ) 
	{
		MIB[1] = HW_NCPU;
		sysctl( MIB, 2, &NumCPU, &Len, NULL, 0 );
		
		if( NumCPU < 1 )
		{
			NumCPU = 1;
		}
	}
	
	BcAssert( NumCPU >= 1 );
	return NumCPU;
}

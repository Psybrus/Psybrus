/**************************************************************************
*
* File:		BcMisc.h
* Author: 	Neil Richardson 
* Ver/Date:	
*			
* Description:
*		Misc functions.
*		
*		
* 
**************************************************************************/

#ifndef __BCMISC_H__
#define __BCMISC_H__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcSleep
void BcSleep( BcReal Seconds );

//////////////////////////////////////////////////////////////////////////
// BcYield
void BcYield();

//////////////////////////////////////////////////////////////////////////
// BcGetHardwareThreadCount
BcU32 BcGetHardwareThreadCount();

//////////////////////////////////////////////////////////////////////////
// BcIsGameThread
BcBool BcIsGameThread();

#endif

/**************************************************************************
*
* File:		cDebug.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Some simple debug stuff for use everywhere.
*		
*		
*		
* 
**************************************************************************/

#include "BcDebug.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include <iostream>

//////////////////////////////////////////////////////////////////////////
//
static BcChar gBuffer_[4096];

//////////////////////////////////////////////////////////////////////////
// BcPrintf
void BcPrintf( const BcChar* pString, ... )
{
	va_list ArgList;
	va_start( ArgList, pString );
	vsprintf( gBuffer_, pString, ArgList );
	va_end( ArgList );

#ifdef PLATFORM_WINDOWS
	OutputDebugString( gBuffer_ );
#endif

	printf( "%s", gBuffer_ );
	//std::cout << gBuffer_ << std::endl;
}

/**************************************************************************
*
* File:		BcString.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "Base/BcString.h"

//////////////////////////////////////////////////////////////////////////
// BcStringPrintf
std::string BcStringPrintf( const BcChar* Format, ... )
{
	BcChar Buffer[ 4096 ];
	va_list ArgList;
	va_start( ArgList, Format );
#if COMPILER_MSVC
	vsprintf_s( Buffer, Format, ArgList );
#else
    vsprintf( Buffer, Format, Args );
#endif
	va_end( ArgList );
	return Buffer;
}



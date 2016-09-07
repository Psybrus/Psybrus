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

#ifndef __BCSTRING_H__
#define __BCSTRING_H__

#include "Base/BcTypes.h"

#include <stdio.h>
#include <string>

//////////////////////////////////////////////////////////////////////////
// BcStrLength
BcU32 BcStrLength( const BcChar* pString );

//////////////////////////////////////////////////////////////////////////
// BcStrStr
BcChar* BcStrStr( BcChar* pStr, const BcChar* pSubStr );
const BcChar* BcStrStr( const BcChar* pStr, const BcChar* pSubStr );


//////////////////////////////////////////////////////////////////////////
// BcStrCopy
void BcStrCopy( BcChar* pDest, BcU32 DestSize, const BcChar* pSrc );

//////////////////////////////////////////////////////////////////////////
// BcStrCompare
BcBool BcStrCompare( const BcChar* pStr1, const BcChar* pStr2 );

//////////////////////////////////////////////////////////////////////////
// BcStrNCompare
BcBool BcStrCompareN( const BcChar* pStr1, const BcChar* pStr2, BcU32 Count );

//////////////////////////////////////////////////////////////////////////
// BcStrCompareCaseInsensitive
BcBool BcStrCompareCaseInsensitive( const BcChar* pStr1, const BcChar* pStr2 );

//////////////////////////////////////////////////////////////////////////
// BcStrCompareCaseInsensitiveN
BcBool BcStrCompareCaseInsensitiveN( const BcChar* pStr1, const BcChar* pStr2, BcU32 Count );

//////////////////////////////////////////////////////////////////////////
// BcSPrintf
#if PLATFORM_WINDOWS || PLATFORM_WINPHONE
#define BcSPrintf		sprintf_s
#else
#define BcSPrintf		snprintf
#endif

//////////////////////////////////////////////////////////////////////////
// BcVSPrintf
#if PLATFORM_WINDOWS || PLATFORM_WINPHONE
#define BcVSPrintf		vsprintf_s
#else
#define BcVSPrintf		vsnprintf
#endif

//////////////////////////////////////////////////////////////////////////
// BcSScanf
#define BcSScanf		sscanf

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
void BcStrReverse( BcChar* pStart, BcChar* pEnd );

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
void BcStrItoa( BcS32 value, BcChar* pStr, BcU32 base = 10 );

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
BcS32 BcStrAtoi( const BcChar* str );

//////////////////////////////////////////////////////////////////////////
// Is string a number?
BcBool BcStrIsNumber( const BcChar* pStr );

//////////////////////////////////////////////////////////////////////////
// BcStrCountChars
BcU32 BcStrCountChars( const BcChar* pStr, BcChar Char );

//////////////////////////////////////////////////////////////////////////
// BcStrReplace
std::string BcStrReplace( const std::string& InString, const std::string& FindString, const std::string& ReplaceString );


#endif


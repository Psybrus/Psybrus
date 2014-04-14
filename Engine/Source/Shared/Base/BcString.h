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

//////////////////////////////////////////////////////////////////////////
// BcStrLength
BcU32 BcStrLength( const BcChar* pString );

//////////////////////////////////////////////////////////////////////////
// BcStrCopy
void BcStrCopy( BcChar* pDest, const BcChar* pSrc );

//////////////////////////////////////////////////////////////////////////
// BcStrStr
BcChar* BcStrStr( BcChar* pStr, const BcChar* pSubStr );
const BcChar* BcStrStr( const BcChar* pStr, const BcChar* pSubStr );


//////////////////////////////////////////////////////////////////////////
// BcStrNCopy
void BcStrCopyN( BcChar* pDest, const BcChar* pSrc, BcU32 Count );

//////////////////////////////////////////////////////////////////////////
// BcStrCompare
BcBool BcStrCompare( const BcChar* pStr1, const BcChar* pStr2 );

//////////////////////////////////////////////////////////////////////////
// BcStrNCompare
BcBool BcStrCompareN( const BcChar* pStr1, const BcChar* pStr2, BcU32 Count );

//////////////////////////////////////////////////////////////////////////
// BcSPrintf
#define BcSPrintf		sprintf

//////////////////////////////////////////////////////////////////////////
// BcSScanf
#define BcSScanf		sscanf

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
void BcStrReverse( BcChar* pStart, BcChar* pEnd );

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
// NOTE: this bad inline, but no cpp?
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

#endif


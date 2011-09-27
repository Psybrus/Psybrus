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

#include "BcTypes.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <cstdlib>

#if PLATFORM_WINDOWS
#define caseInsensitiveComparison stricmp
#define safeCaseInsensitiveComparison strnicmp
#elif PLATFORM_LINUX || PLATFORM_OSX
#define caseInsensitiveComparison strcasecmp
#define safeCaseInsensitiveComparison strncasecmp
#else

#endif


//////////////////////////////////////////////////////////////////////////
// BcStrLength
inline BcU32 BcStrLength( const BcChar* pString )
{
	return static_cast< BcU32 >( strlen( pString ) );
}

//////////////////////////////////////////////////////////////////////////
// BcStrCopy
inline void BcStrCopy( BcChar* pDest, const BcChar* pSrc )
{
	strcpy( pDest, pSrc );
}

//////////////////////////////////////////////////////////////////////////
// BcStrStr
inline BcChar* BcStrStr( BcChar* pStr, const BcChar* pSubStr )
{
	return strstr( pStr, pSubStr );
}

inline const BcChar* BcStrStr( const BcChar* pStr, const BcChar* pSubStr )
{
	return strstr( (char*)pStr, pSubStr );
}


//////////////////////////////////////////////////////////////////////////
// BcStrNCopy
inline void BcStrCopyN( BcChar* pDest, const BcChar* pSrc, BcU32 Count )
{
	strncpy( pDest, pSrc, Count );
	pDest[ Count - 1 ] = '\0';		// Auto termination.
}

//////////////////////////////////////////////////////////////////////////
// BcStrCompare
inline BcBool BcStrCompare( const BcChar* pStr1, const BcChar* pStr2 )
{
	BcU32 Result = caseInsensitiveComparison( pStr1, pStr2 );
	return ( Result == 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcStrNCompare
inline BcBool BcStrCompareN( const BcChar* pStr1, const BcChar* pStr2, BcU32 Count )
{
	BcU32 Result = safeCaseInsensitiveComparison( pStr1, pStr2, Count );
	return ( Result == 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcSPrintf
#define BcSPrintf		sprintf

//////////////////////////////////////////////////////////////////////////
// BcSScanf

#define BcSScanf		sscanf


//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
inline void BcStrReverse( BcChar* pStart, BcChar* pEnd )
{
	char aux;
	while ( pEnd > pStart )
	{
		aux = *pEnd;
		*pEnd-- = *pStart;
		*pStart++ = aux;
	}
}

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
// NOTE: this bad inline, but no cpp?
inline void BcStrItoa( BcS32 value, BcChar* pStr, BcU32 base = 10 )
{
	  static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	  char* wstr = pStr;
	  int sign;

	  // Validate base
	  if ( base < 2 || base > 35 )
	  {
		  *wstr = '\0';
		  return;
	  }

	  // Take care of sign
	  if (( sign = value ) < 0 )
	  {
		  value = -value;
	  }

	  // Conversion. Number is reversed.
	  do
	  {
		  *wstr++ = num[value%base];
	  }
	  while ( value /= base );

	  if ( sign < 0 )
	  {
		  *wstr++ = '-';
	  }
	  *wstr = '\0';

	  // Reverse string
	  BcStrReverse( pStr, wstr - 1 );
}

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
inline BcS32 BcStrAtoi( const BcChar* str )
{
	return atoi( str );
}

#endif


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

#include <cstring>

#if PLATFORM_WINDOWS
#define caseInsensitiveComparison stricmp
#define safeCaseInsensitiveComparison strnicmp
#elif PLATFORM_LINUX || PLATFORM_OSX || PLATFORM_HTML5
#define caseInsensitiveComparison strcasecmp
#define safeCaseInsensitiveComparison strncasecmp
#else

#endif

//////////////////////////////////////////////////////////////////////////
// BcStrLength
BcU32 BcStrLength( const BcChar* pString )
{
	return static_cast< BcU32 >( strlen( pString ) );
}

//////////////////////////////////////////////////////////////////////////
// BcStrCopy
void BcStrCopy( BcChar* pDest, const BcChar* pSrc )
{
	strcpy( pDest, pSrc );
}

//////////////////////////////////////////////////////////////////////////
// BcStrStr
BcChar* BcStrStr( BcChar* pStr, const BcChar* pSubStr )
{
	return strstr( pStr, pSubStr );
}

const BcChar* BcStrStr( const BcChar* pStr, const BcChar* pSubStr )
{
	return strstr( (char*)pStr, pSubStr );
}


//////////////////////////////////////////////////////////////////////////
// BcStrNCopy
void BcStrCopyN( BcChar* pDest, const BcChar* pSrc, BcU32 Count )
{
	strncpy( pDest, pSrc, Count );
	pDest[ Count - 1 ] = '\0';		// Auto termination.
}

//////////////////////////////////////////////////////////////////////////
// BcStrCompare
BcBool BcStrCompare( const BcChar* pStr1, const BcChar* pStr2 )
{
	BcU32 Result = caseInsensitiveComparison( pStr1, pStr2 );
	return ( Result == 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcStrNCompare
BcBool BcStrCompareN( const BcChar* pStr1, const BcChar* pStr2, BcU32 Count )
{
	BcU32 Result = safeCaseInsensitiveComparison( pStr1, pStr2, Count );
	return ( Result == 0 );
}

//////////////////////////////////////////////////////////////////////////
// T.K. - waz 'ere messing with you strings ;-) 
void BcStrReverse( BcChar* pStart, BcChar* pEnd )
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
void BcStrItoa( BcS32 value, BcChar* pStr, BcU32 base )
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
BcS32 BcStrAtoi( const BcChar* str )
{
	return atoi( str );
}

//////////////////////////////////////////////////////////////////////////
// Is string a number?
BcBool BcStrIsNumber( const BcChar* pStr )
{
	BcBool HadDecimal = BcFalse;

	// Strip sign if need be.
	if( *pStr == '+' || *pStr == '-' )
	{
		++pStr;
	}

	// Terminated early, not a number.
	if( *pStr == '\0' )
	{
		return BcFalse;
	}
	
	// Iterate over and check values.
	for( const BcChar* pWStr = pStr; *pWStr != '\0'; ++pWStr )
	{
		// If we find a decimal point, we need to see if we've hit one already, or bail (invalid number).
		if( *pWStr >= '0' && *pWStr <= '9' )
		{
			continue;
		}
		else if( *pWStr == '.' )
		{
			if( HadDecimal == BcFalse )
			{
				HadDecimal = BcTrue;
			}
			else
			{
				return BcFalse;
			}
		}
		else
		{
			return BcFalse;
		}
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// BcStrCountChars
BcU32 BcStrCountChars( const BcChar* pStr, BcChar Char )
{
	BcU32 NoofChars = 0;
	while( *pStr != '\0' )
	{
		if( *pStr++ == Char )
		{
			++NoofChars;
		}
	}
	return NoofChars;
}

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
#include <cstdlib>

#if PLATFORM_WINDOWS || PLATFORM_WINPHONE
#define caseInsensitiveComparison stricmp
#define safeCaseInsensitiveComparison strnicmp
#else
#define caseInsensitiveComparison strcasecmp
#define safeCaseInsensitiveComparison strncasecmp
#endif

//////////////////////////////////////////////////////////////////////////
// BcStrLength
BcU32 BcStrLength( const BcChar* pString )
{
	return static_cast< BcU32 >( strlen( pString ) );
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
// BcStrCopy
void BcStrCopy( BcChar* pDest, size_t DestSize, const BcChar* pSrc )
{
	strncpy( pDest, pSrc, DestSize );
}

//////////////////////////////////////////////////////////////////////////
// BcStrCompare
BcBool BcStrCompare( const BcChar* pStr1, const BcChar* pStr2 )
{
	BcU32 Result = strcmp( pStr1, pStr2 );
	return ( Result == 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcStrCompareN
BcBool BcStrCompareN( const BcChar* pStr1, const BcChar* pStr2, size_t Count )
{
	BcU32 Result = strncmp( pStr1, pStr2, Count );
	return ( Result == 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcStrCompareCaseInsensitive
BcBool BcStrCompareCaseInsensitive( const BcChar* pStr1, const BcChar* pStr2 )
{
	BcU32 Result = caseInsensitiveComparison( pStr1, pStr2 );
	return ( Result == 0 );
}

//////////////////////////////////////////////////////////////////////////
// BcStrCompareCaseInsensitiveN
BcBool BcStrCompareCaseInsensitiveN( const BcChar* pStr1, const BcChar* pStr2, BcU32 Count )
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
//
BcF32 BcStrAtof( const BcChar* str )
{
	return (BcF32)atof( str );
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

//////////////////////////////////////////////////////////////////////////
// BcStrReplace
std::string BcStrReplace( const std::string& InString, const std::string& FindString, const std::string& ReplaceString )
{
	std::string OutString;
	OutString.reserve( InString.size() );

	size_t LastPos = 0;
	size_t FoundPos = 0;
	while( ( FoundPos = InString.find( FindString, LastPos ) ) != std::string::npos )
	{
		OutString.append( InString, LastPos, FoundPos - LastPos );
		OutString.append( ReplaceString );
		LastPos = FoundPos + FindString.size();
	}

	OutString.append( InString.substr( LastPos ) );

	return std::move( OutString );
}
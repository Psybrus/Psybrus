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

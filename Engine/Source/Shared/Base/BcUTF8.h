/**************************************************************************
*
* File:		BcUTF8.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		UTF8 routines
*		
*
*
* 
**************************************************************************/

#ifndef __BCUTF8_H__
#define __BCUTF8_H__

#include "BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcUTF8
class BcUTF8
{
public:
	static BcU32 toUCS4( const BcChar* pChar, BcU32& OutChar );
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcU32 BcUTF8::toUCS4( const BcChar* pChar, BcU32& OutChar )
{
	if ( ( pChar[0] & 0x80 ) == 0x00 )
	{
		OutChar = ( pChar[0] & 0x7f );
		return 1;
	}
	else if ( ( pChar[0] & 0xe0 ) == 0xc0 )
	{
		OutChar = ( ( pChar[0] & 0x1f ) << 6 ) +
		          ( ( pChar[1] & 0x3f ) );
		return 2;
	}
	else if ( ( pChar[0] & 0xf0 ) == 0xe0 )
	{
		OutChar = ( ( pChar[0] & 0x0f ) << 12 ) +
		          ( ( pChar[1] & 0x3f ) << 6 ) +
		          ( ( pChar[2] & 0x3f ) );
		return 3;
	}
	else if ( ( pChar[0] & 0xf8 ) == 0xf0 )
	{
		OutChar = ( ( pChar[0] & 0x07 ) << 18 ) +
		          ( ( pChar[1] & 0x3f ) << 12 ) +
		          ( ( pChar[2] & 0x3f ) << 6 ) +
		          ( ( pChar[3] & 0x3f ) );
		return 4;
	}

	OutChar = 0;
	return 1;
}

/*
	Reference:
	10000000	0x80
	11000000	0xc0
	11100000	0xe0
	11110000	0xf0
	00111111	0x3f
	00011111	0x1f
	00001111	0x0f
	00000111	0x07
*/

#endif

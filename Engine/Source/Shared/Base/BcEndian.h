/**************************************************************************
*
* File:		BcEndian.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Manual and Automatic Endian Conversion.
*		Stores internally as big endian (network byte order).
*
*
* 
**************************************************************************/

#ifndef __BCENDIAN_H__
#define __BCENDIAN_H__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// Utility
BcForceInline BcU16 BcFlipEndian( BcU16 Value )
{
	const BcU8* pValue = (const BcU8*)&Value;
	return pValue[ 0 ] << 8 | pValue[ 1 ];
}

BcForceInline BcU32 BcFlipEndian( BcU32 Value )
{
	const BcU8* pValue = (const BcU8*)&Value;
	return pValue[ 0 ] << 24 | pValue[ 1 ] << 16 | pValue[ 2 ] << 8 | pValue[ 3 ];
}

BcForceInline BcF32 BcFlipEndian( BcF32 Value )
{
	BcAssert( sizeof( BcF32 ) == 4 );
	const BcU8* pValue = (const BcU8*)&Value;
	BcU32 ConvertedValue = pValue[ 0 ] << 24 | pValue[ 1 ] << 16 | pValue[ 2 ] << 8 | pValue[ 3 ];
	return ( *( (BcF32*)&ConvertedValue ) );
}

//////////////////////////////////////////////////////////////////////////
// Macros
#if PSY_ENDIAN_BIG
// Convert from or to little endian to native.
#  define BcLittleEndian( a )		BcFlipEndian( a )

// Convert from or to big endian to native.
#  define BcBigEndian( a )			( a )

#else
// Convert from or to little endian to native.
#  define BcLittleEndian( a )		( a )

// Convert from or to big endian to native.
#  define BcBigEndian( a )			BcFlipEndian( a )

#endif

//////////////////////////////////////////////////////////////////////////
// BcEndianValue
template< typename _Ty >
class BcEndianValue
{
public:
	void operator = ( _Ty Value );
	operator _Ty() const;
	
	template< typename _Uy >
	BcInline operator _Uy() const
	{
		_Ty Intermediate = operator _Ty();
		return (_Uy)Intermediate;
	}

private:
	_Ty Value_;			// Big endian.
};

template< typename _Ty >
BcInline void BcEndianValue< _Ty >::operator = ( _Ty Value )
{
	Value_ = BcBigEndian( Value );
}

template< typename _Ty >
BcInline BcEndianValue< _Ty >::operator _Ty() const
{
	return BcBigEndian( Value_ );
}

#endif

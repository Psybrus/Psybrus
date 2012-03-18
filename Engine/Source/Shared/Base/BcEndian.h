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
#include "Base/BcVectors.h"
#include "Base/BcMat3d.h"
#include "Base/BcMat4d.h"
#include "Base/BcQuat.h"

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

BcForceInline BcReal BcFlipEndian( BcReal Value )
{
	BcAssert( sizeof( BcReal ) == 4 );
	const BcU8* pValue = (const BcU8*)&Value;
	BcU32 ConvertedValue = pValue[ 0 ] << 24 | pValue[ 1 ] << 16 | pValue[ 2 ] << 8 | pValue[ 3 ];
	return ( *( (BcReal*)&ConvertedValue ) );
}

BcInline BcVec2d BcFlipEndian( const BcVec2d& Value )
{
	return BcVec2d( BcFlipEndian( Value.x() ),
	                BcFlipEndian( Value.y() ) );
}

BcInline BcVec3d BcFlipEndian( const BcVec3d& Value )
{
	return BcVec3d( BcFlipEndian( Value.x() ),
	                BcFlipEndian( Value.y() ),
	                BcFlipEndian( Value.z() ) );
}

BcInline BcVec4d BcFlipEndian( const BcVec4d& Value )
{
	return BcVec4d( BcFlipEndian( Value.x() ),
	                BcFlipEndian( Value.y() ),
	                BcFlipEndian( Value.z() ),
	                BcFlipEndian( Value.w() ) );
}

BcInline BcQuat BcFlipEndian( const BcQuat& Value )
{
	return BcQuat( BcFlipEndian( Value.x() ),
	               BcFlipEndian( Value.y() ),
	               BcFlipEndian( Value.z() ),
	               BcFlipEndian( Value.w() ) );
}

BcInline BcMat3d BcFlipEndian( const BcMat3d& Value )
{
	return BcMat3d( BcFlipEndian( Value.row0() ),
	                BcFlipEndian( Value.row1() ),
	                BcFlipEndian( Value.row2() ) );
}

BcInline BcMat4d BcFlipEndian( const BcMat4d& Value )
{
	return BcMat4d( BcFlipEndian( Value.row0() ),
	                BcFlipEndian( Value.row1() ),
	                BcFlipEndian( Value.row2() ),
	                BcFlipEndian( Value.row3() ) );
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

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef BcEndianValue< BcU16 >		BcEndianU16;
typedef BcEndianValue< BcU32 >		BcEndianU32;
typedef BcEndianValue< BcReal >		BcEndianReal;
typedef BcEndianValue< BcVec2d >	BcEndianVec2d;
typedef BcEndianValue< BcVec3d >	BcEndianVec3d;
typedef BcEndianValue< BcVec4d >	BcEndianVec4d;
typedef BcEndianValue< BcQuat >		BcEndianQuat;
typedef BcEndianValue< BcMat3d >	BcEndianMat3d;
typedef BcEndianValue< BcMat4d >	BcEndianMat4d;

#endif

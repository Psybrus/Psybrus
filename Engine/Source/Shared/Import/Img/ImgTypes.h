/**************************************************************************
*
* File:		ImgTypes.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*
*		
*
*
* 
**************************************************************************/

#ifndef __IMGTYPES_H__
#define __IMGTYPES_H__

//////////////////////////////////////////////////////////////////////////
// Includes
#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcMath.h"
#include "Base/BcMemory.h"

#include <vector>
#include <memory>

//////////////////////////////////////////////////////////////////////////
// ImgEncodeFormat
enum class ImgEncodeFormat
{
	UNKNOWN,

	I8,
	A8,
	R8G8B8A8,

	BC1,
	BC2,
	BC3,
	BC4,
	BC5,

	ETC1,
};

//////////////////////////////////////////////////////////////////////////
// ImgColourf
struct ImgColourf
{
	BcF32 R_, G_, B_, A_;

	struct ImgColour toGamma( BcF32 GammaRGB ) const;
};

//////////////////////////////////////////////////////////////////////////
// ImgColour
struct ImgColour
{
	BcU8 R_, G_, B_, A_;
	
	bool operator == ( const ImgColour& Other ) const
	{
		return R_ == Other.R_ && G_ == Other.G_ && B_ == Other.B_ && A_ == Other.A_;
	}

	bool operator != ( const ImgColour& Other ) const
	{
		return R_ != Other.R_ || G_ != Other.G_ || B_ != Other.B_ || A_ != Other.A_;
	}

	struct ImgColourf toLinear( BcF32 GammaRGB ) const;
};

//////////////////////////////////////////////////////////////////////////
// toGamma
inline ImgColour ImgColourf::toGamma( BcF32 GammaRGB ) const
{
	ImgColour RetVal;
	RetVal.R_ = static_cast< BcU8 >( std::powf( BcClamp( R_, 0.0f, 1.0f ), 1.0f / GammaRGB ) * 255.0f ); 
	RetVal.G_ = static_cast< BcU8 >( std::powf( BcClamp( G_, 0.0f, 1.0f ), 1.0f / GammaRGB ) * 255.0f );
	RetVal.B_ = static_cast< BcU8 >( std::powf( BcClamp( B_, 0.0f, 1.0f ), 1.0f / GammaRGB ) * 255.0f );
	RetVal.A_ = static_cast< BcU8 >( BcClamp( A_, 0.0f, 1.0f ) * 255.0f );
	return RetVal;
	
#if 0 // TODO
	ImgColourf sRGBLo =
	{
		R_ * 12.92f,
		G_ * 12.92f,
		B_ * 12.92f,
		1.0f,
	};

	ImgColourf sRGBHi =
	{
		std::powf( std::abs( R_ ), 1.0f / 2.4f ) * 1.055f - 0.055f,
		std::powf( std::abs( G_ ), 1.0f / 2.4f ) * 1.055f - 0.055f,
		std::powf( std::abs( B_ ), 1.0f / 2.4f ) * 1.055f - 0.055f,
		1.0f,
	};
	
	ImgColourf sRGB =
	{
		( R_ <= 0.0031308f ) ? sRGBLo.R_ : sRGBHi.R_,
		( G_ <= 0.0031308f ) ? sRGBLo.G_ : sRGBHi.G_,
		( B_ <= 0.0031308f ) ? sRGBLo.B_ : sRGBHi.B_,
		1.0f
	};

	ImgColour RetVal;
	RetVal.R_ = static_cast< BcU8 >( BcClamp( sRGB.R_, 0.0f, 1.0f ) * 255.0f ); 
	RetVal.G_ = static_cast< BcU8 >( BcClamp( sRGB.G_, 0.0f, 1.0f ) * 255.0f );
	RetVal.B_ = static_cast< BcU8 >( BcClamp( sRGB.B_, 0.0f, 1.0f ) * 255.0f );
	RetVal.A_ = static_cast< BcU8 >( BcClamp( A_, 0.0f, 1.0f ) * 255.0f );
	return RetVal;
#endif
}

//////////////////////////////////////////////////////////////////////////
// toLinear
inline ImgColourf ImgColour::toLinear( BcF32 GammaRGB ) const
{
	ImgColourf RetVal;
	RetVal.R_ = std::powf( static_cast< BcF32 >( R_ ) / 255.0f, GammaRGB ); 
	RetVal.G_ = std::powf( static_cast< BcF32 >( G_ ) / 255.0f, GammaRGB ); 
	RetVal.B_ = std::powf( static_cast< BcF32 >( B_ ) / 255.0f, GammaRGB ); 
	RetVal.A_ = static_cast< BcF32 >( A_ ) / 255.0f;
	return RetVal;
	
#if 0 // TODO
	const BcF32 R = BcClamp( static_cast< BcF32 >( R_ ) / 255.0f, 0.0f, 1.0f );
	const BcF32 G = BcClamp( static_cast< BcF32 >( G_ ) / 255.0f, 0.0f, 1.0f );
	const BcF32 B = BcClamp( static_cast< BcF32 >( B_ ) / 255.0f, 0.0f, 1.0f );


	ImgColourf LinearLo =
	{
		R / 12.92f,
		G / 12.92f,
		B / 12.92f,
		1.0f,
	};

	ImgColourf LinearHi =
	{
		std::powf( ( R + 0.055f ) / 1.055f, 2.4f ),
		std::powf( ( G + 0.055f ) / 1.055f, 2.4f ),
		std::powf( ( B + 0.055f ) / 1.055f, 2.4f ),
		1.0f,
	};
	
	ImgColourf Linear =
	{
		( R_ <= 0.04045f ) ? LinearLo.R_ : LinearHi.R_,
		( G_ <= 0.04045f ) ? LinearLo.G_ : LinearHi.G_,
		( B_ <= 0.04045f ) ? LinearLo.B_ : LinearHi.B_,
		1.0f
	};

	ImgColourf RetVal;
	RetVal.R_ = Linear.R_;
	RetVal.G_ = Linear.G_;
	RetVal.B_ = Linear.B_;
	RetVal.A_ = A_;
	return RetVal;
#endif
}

//////////////////////////////////////////////////////////////////////////
// ImgRect
struct ImgRect
{
	BcU32 X_;
	BcU32 Y_;
	BcU32 W_;
	BcU32 H_;
	
	BcBool isInsideOf( const ImgRect& Other ) const
	{
		if( ( X_ >= Other.X_ && ( X_ + W_ ) <= ( Other.X_ + Other.W_  ) ) &&
		    ( Y_ >= Other.Y_ && ( Y_ + H_ ) <= ( Other.Y_ + Other.H_  ) ) )
		{
			return BcTrue;
		}
		return BcFalse;
	}
	
	BcBool isOutsideOf( const ImgRect& Other ) const
	{
		if( ( ( X_ + W_ ) < Other.X_ || X_ > ( Other.X_ + Other.W_ ) ) ||
		    ( ( Y_ + H_ ) < Other.Y_ || Y_ > ( Other.Y_ + Other.H_ ) ) )
		{
			return BcTrue;
		}
		return BcFalse;
	}
};

typedef std::vector< ImgRect > ImgRectList;
typedef ImgRectList::iterator ImgRectListIterator;
typedef ImgRectList::const_iterator ImgRectListConstIterator;


#endif

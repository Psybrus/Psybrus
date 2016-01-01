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
// ImgEncodeFormat - Must match RsTextureFormat 1:1.
enum class ImgEncodeFormat
{
	UNKNOWN,

	// Colour.
	R8,
	R8G8,
	R8G8B8,
	R8G8B8A8,
	R16F,
	R16FG16F,
	R16FG16FB16F,
	R16FG16FB16FA16F,
	R32F,
	R32FG32F,
	R32FG32FB32F,
	R32FG32FB32FA32F,
	R10G10B10A2,
	R11G11B10F,
	DXT1,
	DXT3,
	DXT5,
	ETC1,

	// Depth stencil.
	D16,
	D24,
	D32,
	D24S8,
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

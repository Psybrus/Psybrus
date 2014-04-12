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
#include "Base/BcMemory.h"
#include "Base/BcString.h"
#include "Base/BcMath.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// ImgEncodeFormat
enum ImgEncodeFormat
{
	imgEF_RGB8,
	imgEF_RGBA8,
	imgEF_BGR8,
	imgEF_ABGR8,
	imgEF_I8,
	imgEF_DXT1,
	imgEF_DXT3,
	imgEF_DXT5
};

//////////////////////////////////////////////////////////////////////////
// ImgColour
struct ImgColour
{
	BcU8		R_;
	BcU8		G_;
	BcU8		B_;
	BcU8		A_;
	
	bool operator == ( const ImgColour& Other ) const
	{
		return R_ == Other.R_ && G_ == Other.G_ && B_ == Other.B_ && A_ == Other.A_;
	}

	bool operator != ( const ImgColour& Other ) const
	{
		return R_ != Other.R_ || G_ != Other.G_ || B_ != Other.B_ || A_ != Other.A_;
	}
};

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

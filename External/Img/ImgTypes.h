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
#include "BcTypes.h"
#include "BcDebug.h"
#include "BcMemory.h"
#include "BcString.h"
#include "BcMath.h"

//////////////////////////////////////////////////////////////////////////
// ImgColour
struct ImgColour
{
	BcU8		B_;
	BcU8		G_;
	BcU8		R_;
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


//////////////////////////////////////////////////////////////////////////
// Format
enum eImgFormat
{
	imgFMT_RGB = 0,
	imgFMT_RGBA,
	imgFMT_INDEXED,
};

#endif

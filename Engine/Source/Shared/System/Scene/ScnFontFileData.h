/**************************************************************************
*
* File:		ScnFontFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNFONTFILEDATA__
#define __SCNFONTFILEDATA__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnFontHeader
struct ScnFontHeader
{
	BcU32							NoofGlyphs_;
	BcU32							TextureRef_;
	BcF32							NominalSize_;
};

//////////////////////////////////////////////////////////////////////////
// ScnFontGlyphDesc
struct ScnFontGlyphDesc
{	
	// Texture.
	BcF32							UA_;
	BcF32							VA_;
	BcF32							UB_;
	BcF32							VB_;
	
	// Positioning.
	BcF32							OffsetX_;
	BcF32							OffsetY_;
	BcF32							Width_;
	BcF32							Height_;
	BcF32							AdvanceX_;
		
	// CharCode
	BcU32							CharCode_;
};

#endif

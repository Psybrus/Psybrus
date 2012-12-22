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
	BcReal							NominalSize_;
};

//////////////////////////////////////////////////////////////////////////
// ScnFontGlyphDesc
struct ScnFontGlyphDesc
{	
	// Texture.
	BcReal							UA_;
	BcReal							VA_;
	BcReal							UB_;
	BcReal							VB_;
	
	// Positioning.
	BcReal							OffsetX_;
	BcReal							OffsetY_;
	BcReal							Width_;
	BcReal							Height_;
	BcReal							AdvanceX_;
		
	// CharCode
	BcU32							CharCode_;
};

#endif

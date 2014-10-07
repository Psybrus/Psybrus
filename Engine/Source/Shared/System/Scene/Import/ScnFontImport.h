/**************************************************************************
*
* File:		ScnFontImport.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNFONTIMPORT_H__
#define __SCNFONTIMPORT_H__

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

#include "Base/BcStream.h"

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// ScnFontImport
class ScnFontImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnFontImport, CsResourceImporter );

public:
	ScnFontImport();
	ScnFontImport( ReNoInit );
	virtual ~ScnFontImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	ImgImageUPtr makeImageForGlyphMono( struct FT_GlyphRec_* Glyph, BcU32 BorderSize );
	ImgImageUPtr makeImageForGlyphNormal( struct FT_GlyphRec_* Glyph, BcU32 BorderSize );

private:
	std::string Source_;
	BcU32 NominalSize_;
	BcBool DistanceField_;
	BcU32 Spread_;
	
};

#endif

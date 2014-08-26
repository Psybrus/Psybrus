/**************************************************************************
*
* File:		ScnFontImport.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNFONTIMPORT_H__
#define __SCNFONTIMPORT_H__

#ifdef PSY_SERVER
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
	REFLECTION_DECLARE_DERIVED( ScnFontImport, CsResourceImporter );

public:
	ScnFontImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	ImgImageUPtr ScnFontImport::makeImageForGlyphMono( struct FT_GlyphRec_* Glyph, BcU32 BorderSize );
	ImgImageUPtr ScnFontImport::makeImageForGlyphNormal( struct FT_GlyphRec_* Glyph, BcU32 BorderSize );

private:
	std::string Source_;

};

#endif
#endif

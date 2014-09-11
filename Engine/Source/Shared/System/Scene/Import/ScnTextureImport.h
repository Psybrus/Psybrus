/**************************************************************************
*
* File:		ScnTextureImport.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnTextureImport_H__
#define __ScnTextureImport_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"
#include "System/Renderer/RsTypes.h"
#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureImport
class ScnTextureImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnTextureImport, CsResourceImporter );

public:
	ScnTextureImport();
	ScnTextureImport( ReNoInit );
	ScnTextureImport( 
		const std::string Name,
		const std::string Type,
		const std::string Source,
		RsTextureFormat Format );
	virtual ~ScnTextureImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& );

private:
	std::vector< std::string > Source_;
	RsTextureFormat Format_;  // TODO: Use.
	RsColour ClearColour_;
	BcBool AlphaFromIntensity_;
	BcBool DistanceField_;
	BcU32 Spread_;
	BcBool TileAtlas_;
	BcU32 TileWidth_;
	BcU32 TileHeight_;
	
	RsTextureType TextureType_;
	BcU32 Width_;
	BcU32 Height_;
	BcU32 Depth_;
	BcU32 Levels_;
};

#endif
#endif

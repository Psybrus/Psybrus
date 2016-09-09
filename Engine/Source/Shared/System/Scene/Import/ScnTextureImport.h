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

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"
#include "System/Renderer/RsColour.h"
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
		RsResourceFormat Format,
		ImgEncodeFormat EncodeFormat = ImgEncodeFormat::UNKNOWN );
	ScnTextureImport( 
		const std::string Name,
		const std::string Type,
		const std::string Source,
		RsResourceFormat Format,
		BcU32 TileWidth,
		BcU32 TileHeight );
	virtual ~ScnTextureImport();

	/**
	 * Import.
	 */
	BcBool import() override;

	/**
	 * Set format.
	 */
	void setFormat( RsResourceFormat Format ) { Format_ = Format; }

private:
	bool loadDDS( const char* FileName );
	ImgImageList loadImages( std::vector< std::string > Sources );

	ImgImageUPtr processAlphaFromIntensity( ImgImageUPtr Image );
	ImgImageUPtr processDistanceField( ImgImageUPtr Image, BcU32 Spread );
	ImgImageUPtr processRoundUpPot( ImgImageUPtr Image );
	ImgImageUPtr processRoundDownPot( ImgImageUPtr Image );

	ImgImageList generateMipMaps( ImgImageUPtr Image );


private:
	std::vector< std::string > Source_;
	RsResourceFormat Format_;  // TODO: Use.
	ImgEncodeFormat EncodeFormat_;
	BcBool RenderTarget_;
	BcBool DepthStencilTarget_;
	RsColour ClearColour_;
	BcBool AlphaFromIntensity_;
	BcBool DistanceField_;
	BcU32 Spread_;
	BcU32 Border_;
	BcBool TileAtlas_;
	BcU32 TileWidth_;
	BcU32 TileHeight_;
	BcBool TrimMipTails_;
	BcBool RoundUpPowerOfTwo_;
	BcBool RoundDownPowerOfTwo_;
	
	RsTextureType TextureType_;
	BcS32 Width_;
	BcS32 Height_;
	BcU32 Depth_;
	BcU32 Levels_;
};

#endif

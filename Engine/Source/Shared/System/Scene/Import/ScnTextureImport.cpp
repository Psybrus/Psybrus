/**************************************************************************
*
* File:		ScnTextureImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnTextureImport.h"
#include "System/Scene/Rendering/ScnTextureFileData.h"
#include "System/Scene/Rendering/ScnTextureAtlasFileData.h"

#include "System/Renderer/RsTypes.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcMath.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnTextureImport )
	
void ScnTextureImport::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnTextureImport::Source_ ),
	};
	*/
		
	ReRegisterClass< ScnTextureImport, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTextureImport::ScnTextureImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnTextureImport::~ScnTextureImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnTextureImport::import(
		const Json::Value& Object )
{
	Json::Value Source = Object[ "source" ];
	const Json::Value& Format = Object[ "format" ];
	const Json::Value& ClearColourValue = Object[ "clearcolour" ];
	const Json::Value& DistanceFieldValue = Object[ "distancefield" ];
	const Json::Value& SpreadValue = Object[ "spread" ];
	const Json::Value& AlphaFromIntensityValue = Object[ "alphafromintensity" ];
	ImgColour ClearColour = { 0, 0, 0, 0 };

	if( ClearColourValue.type() != Json::nullValue )
	{
		RsColour Colour = ClearColourValue.asCString();
		ClearColour.R_ = BcU8( BcClamp( BcU32( Colour.r() * 255.0f ), 0, 255 ) );
		ClearColour.G_ = BcU8( BcClamp( BcU32( Colour.g() * 255.0f ), 0, 255 ) );
		ClearColour.B_ = BcU8( BcClamp( BcU32( Colour.b() * 255.0f ), 0, 255 ) );
		ClearColour.A_ = BcU8( BcClamp( BcU32( Colour.a() * 255.0f ), 0, 255 ) );
	}

	BcBool DistanceField = ( DistanceFieldValue.type() != Json::nullValue ) ? DistanceFieldValue.asBool() : BcFalse;
	BcU32 Spread = ( SpreadValue.type() != Json::nullValue ) ? SpreadValue.asUInt() : 0;
	BcU32 SpreadDouble = Spread * 2;
	BcBool AlphaFromIntensity = ( AlphaFromIntensityValue.type() != Json::nullValue ) ? AlphaFromIntensityValue.asBool() : BcFalse;

	// Texture atlas.
	if( Source.isArray() )
	{

		// Load all source images.
		ImgImageList ImageList;
		
		for( BcU32 Idx = 0; Idx < Source.size(); ++Idx )
		{
			std::string FileName = Source[ Idx ].asString();  

			// Add as dependancy.
			CsResourceImporter::addDependency( FileName.c_str() );

			// Load image.
			ImgImageUPtr pImage = Img::load( FileName.c_str() );

			// Generate alpha from intensity.
			if( pImage != NULL && AlphaFromIntensity )
			{
				for( BcU32 Y = 0; Y < pImage->width(); ++Y )
				{
					for( BcU32 X = 0; X < pImage->height(); ++X )
					{
						ImgColour Texel = pImage->getPixel( X, Y );
						Texel.A_ = (BcU8)( ( (BcU32)Texel.R_ + (BcU32)Texel.G_ + (BcU32)Texel.B_ ) / 3.0f );
						pImage->setPixel( X, Y, Texel );
					}
				}
			}

			// Replace with a distance field version.
			if( pImage != NULL && DistanceField == BcTrue )
			{
				ImgImageUPtr pPaddedImage = ImgImageUPtr( new ImgImage() );

				ImgColour FillColour = { 0, 0, 0, 0 };
				BcU32 NewWidth = BcPotNext( pImage->width() + SpreadDouble );
				BcU32 NewHeight = BcPotNext( pImage->height() + SpreadDouble );
				
				ImgRect SrcRect = { 0, 0, pImage->width(), pImage->height() };
				ImgRect DstRect = { Spread, Spread, pImage->width(), pImage->height() };
				
				pPaddedImage->create( NewWidth, NewHeight, &FillColour );
				pPaddedImage->blit( pImage.get(), SrcRect, DstRect );
								
				// Distance field.
				ImgImageUPtr pDistanceFieldImage = pPaddedImage->generateDistanceField( 128, (BcF32)Spread );
								
				// Scale down 8x.
				ImgImageUPtr pScale1_2 = pDistanceFieldImage->resize( NewWidth >> 1, NewHeight >> 1 );
				ImgImageUPtr pScale1_4 = pScale1_2->resize( NewWidth >> 2, NewHeight >> 2 );			
							
				// Crop to final size.
				ImgImageUPtr pFinal = pScale1_4->canvasSize( ( ( pImage->width() + SpreadDouble ) / 4 ), ( ( pImage->height() + SpreadDouble ) / 4 ), &FillColour );

				// Store image.
				pImage = std::move( pFinal );
			}
			
			// Add to list (even if null).
			ImageList.push_back( std::move( pImage ) );
		}

		// If we have images, generate an atlas and export.
		if( ImageList.size() > 0 )
		{
			BcStream HeaderStream;
			BcStream RectsStream;
						
			// Create an atlas of all source textures..
			ImgRectList RectList;
			ImgImageUPtr pAtlasImage = ImgImage::generateAtlas( ImageList, RectList, 256, 256, ClearColour );
						
			// Setup header.
			ScnTextureAtlasHeader Header = 
			{
				(BcU32)ImageList.size()
			};

			HeaderStream << Header;
						
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				ImgRect& Rect = RectList[ Idx ];
				ScnTextureAtlasRect OutRect = 
				{
					{
						BcF32( Rect.X_ + ( Spread / 4 ) ) / BcF32( pAtlasImage->width() ),
						BcF32( Rect.Y_ + ( Spread / 4 ) ) / BcF32( pAtlasImage->height() ),
						BcF32( Rect.W_ - ( SpreadDouble / 4 ) ) / BcF32( pAtlasImage->width() ),
						BcF32( Rect.H_ - ( SpreadDouble / 4 ) ) / BcF32( pAtlasImage->height() )
					}
				};
				
				RectsStream << OutRect;
			}
			
			// Add chunks.
			CsResourceImporter::addChunk( BcHash( "atlasheader" ), HeaderStream.pData(), HeaderStream.dataSize() );
			CsResourceImporter::addChunk( BcHash( "atlasrects" ), RectsStream.pData(), RectsStream.dataSize() );

			// NOTE: Need a better solution for this. Don't want to reimport this texture.
			CsDependencyList TextureDependancyList;

			// Create a texture.
			std::string AtlasName = Object[ "name" ].asString() + "textureatlas";
			std::string AtlasFileName = getIntermediatePath() + std::string( "/" ) + AtlasName + ".png";
			Img::save( AtlasFileName.c_str(), pAtlasImage.get() );
			
			ImageList.clear();
			pAtlasImage = nullptr;

			// Fall through to the next section.
			// TODO: Refactor so we don't do it this awful way.
			Source = AtlasFileName;

			// Don't do distance field when falling through.
			DistanceField = BcFalse;
		}
	}

	if( Source.type() == Json::stringValue )
	{
		const std::string& FileName = Source.asString();

		// Add root dependency.
		CsResourceImporter::addDependency( FileName.c_str() );

		// Load texture from file and create the data for export.
		ImgImageList MipImages;
		MipImages.push_back( Img::load( FileName.c_str() ) );

		// Replace with a distance field version.
		if( MipImages[ 0 ] != nullptr && DistanceField == BcTrue )
		{
			ImgImageUPtr pPaddedImage = ImgImageUPtr( new ImgImage() );

			ImgColour FillColour = { 0, 0, 0, 0 };
			BcU32 NewWidth = BcPotNext( MipImages[ 0 ]->width() + SpreadDouble );
			BcU32 NewHeight = BcPotNext( MipImages[ 0 ]->height() + SpreadDouble );
				
			ImgRect SrcRect = { 0, 0, MipImages[ 0 ]->width(), MipImages[ 0 ]->height() };
			ImgRect DstRect = { Spread, Spread, MipImages[ 0 ]->width(), MipImages[ 0 ]->height() };
				
			pPaddedImage->create( NewWidth, NewHeight, &FillColour );
			pPaddedImage->blit( MipImages[ 0 ].get(), SrcRect, DstRect );
								
			// Distance field.
			ImgImageUPtr pDistanceFieldImage = pPaddedImage->generateDistanceField( 128, (BcF32)Spread );
								
			// Scale down 8x.
			ImgImageUPtr pScale1_2 = pDistanceFieldImage->resize( NewWidth >> 1, NewHeight >> 1 );
			ImgImageUPtr pScale1_4 = pScale1_2->resize( NewWidth >> 2, NewHeight >> 2 );			
							
			// Store image.
			MipImages[ 0 ] = std::move( pScale1_4 );
		}


		// TODO: Throw exception instead on failure.
		if( MipImages[ 0 ] != nullptr )
		{
			BcU32 W = MipImages[ 0 ]->width();
			BcU32 H = MipImages[ 0 ]->height();
					
			// Downsample texture for mip maps.
			if( BcPot( W ) && BcPot( H ) )
			{
				// Down to a minimum of 4x4.
				while( W > 4 && H > 4 )
				{
					W >>= 1;
					H >>= 1;
					MipImages.push_back( MipImages[ MipImages.size() - 1 ]->resize( W, H ) );
				}
			}

			// Encode the image as a format.
			BcU8* pEncodedImageData = NULL;
			BcU32 EncodedImageDataSize = 0;

			// TODO: Take from parameters.
			ImgEncodeFormat EncodeFormat = imgEF_RGBA8;
			RsTextureFormat TextureFormat = RsTextureFormat::R8G8B8A8;
			RsTextureType TextureType = MipImages[ 0 ]->height() == 0 ? RsTextureType::TEX1D : RsTextureType::TEX2D;
		
			// Use tex compression unless in debug.
	#if !PSY_DEBUG
			if( TextureType == RsTextureType::TEX2D )
			{
				if( Format.type() == Json::nullValue &&
					pTopLevelImage->width() % 4 == 0 && 
					pTopLevelImage->height() % 4 == 0 )
				{
					if( pImage->hasAlpha( 8 ) == BcFalse )
					{
						EncodeFormat = imgEF_DXT1;
						TextureFormat = RsTextureFormat::DXT1;
					}
					else
					{
						EncodeFormat = imgEF_DXT5;
						TextureFormat = RsTextureFormat::DXT5;
					}
				}
				else
				{
					// HACK.
				}
			}
	#endif
			// Streams.
			BcStream HeaderStream;
			BcStream BodyStream( BcFalse, 1024, EncodedImageDataSize );

			// Write header.
			ScnTextureHeader Header = 
			{ 
				MipImages[ 0 ]->width(), 
				MipImages[ 0 ]->height(), 
				0,
				(BcU32)MipImages.size(), 
				TextureType, 
				TextureFormat, 
				BcFalse 
			};	// TODO: Take type from file.
			HeaderStream << Header;

			// Write all mip images into the same body for now.
			for( BcU32 Idx = 0; Idx < MipImages.size(); ++Idx )
			{
				auto* pImage = MipImages[ Idx ].get();
				if( pImage->encodeAs( EncodeFormat, pEncodedImageData, EncodedImageDataSize ) )
				{
					// Serialize encoded images.
					BodyStream.push( pEncodedImageData, EncodedImageDataSize );
					delete [] pEncodedImageData;
					pEncodedImageData = NULL;
					EncodedImageDataSize = 0;
				}
				else
				{
					BcPrintf( "Failed to encode image \"%s\"\n", FileName.c_str() );
				}
			}

			// Clear images.
			MipImages.clear();

			// Add chunks.
			CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
			CsResourceImporter::addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );

			return BcTrue;
		}
		else
		{
			BcPrintf( "Failed to load image \"%s\"\n", FileName.c_str() );
		}
	}
	else
	{
		// User created texture.
		BcU32 Width = 0;
		BcU32 Height = 0;
		BcU32 Depth = 0;

		RsTextureType TextureType;

		const Json::Value& Type = Object[ "texturetype" ];
		const Json::Value& WidthValue = Object[ "width" ];
		const Json::Value& HeightValue = Object[ "height" ];
		const Json::Value& DepthValue = Object[ "depth" ];

		if( BcStrCompare( Type.asCString(), "1d" ) )
		{
			TextureType = RsTextureType::TEX1D;
			Width = WidthValue.asUInt();
		}
		else if( BcStrCompare( Type.asCString(), "2d" ) )
		{
			TextureType = RsTextureType::TEX2D;
			Width = WidthValue.asUInt();
			Height = HeightValue.asUInt();
		}
		else if( BcStrCompare( Type.asCString(), "3d" ) )
		{
			TextureType = RsTextureType::TEX3D;
			Width = WidthValue.asUInt();
			Height = HeightValue.asUInt();
			Depth = DepthValue.asUInt();
		}


		ScnTextureHeader Header = { Width, Height, Depth, 1, TextureType, RsTextureFormat::R8G8B8A8, BcTrue };
		BcStream HeaderStream;
		HeaderStream << Header;
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
		return BcTrue;
	}

	return BcFalse;
}

#endif

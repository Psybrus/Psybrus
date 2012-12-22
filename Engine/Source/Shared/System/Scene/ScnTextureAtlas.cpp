/**************************************************************************
*
* File:		ScnTextureAtlas.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnTextureAtlas.h"

#ifdef PSY_SERVER
#include "Base/BcStream.h"
#include "Import/Img/Img.h"
#include "System/Content/CsPackageImporter.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnTextureAtlas::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	const Json::Value& Source = Object[ "source" ];
	const Json::Value& ClearColourValue = Object[ "clearcolour" ];
	const Json::Value& DistanceFieldValue = Object[ "distancefield" ];
	const Json::Value& SpreadValue = Object[ "spread" ];
	const Json::Value& AlphaFromIntensityValue = Object[ "alphafromintensity" ];
	if( Source.isArray() )
	{
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

		// Load all source images.
		ImgImageList ImageList;
		
		for( BcU32 Idx = 0; Idx < Source.size(); ++Idx )
		{
			std::string FileName = Source[ Idx ].asString();  

			// Add as dependancy.
			Importer.addDependency( FileName.c_str() );

			// Load image.
			ImgImage* pImage = Img::load( FileName.c_str() );

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
				ImgImage* pPaddedImage = new ImgImage();

				ImgColour FillColour = { 0, 0, 0, 0 };
				BcU32 NewWidth = BcPotNext( pImage->width() + SpreadDouble );
				BcU32 NewHeight = BcPotNext( pImage->height() + SpreadDouble );
				
				ImgRect SrcRect = { 0, 0, pImage->width(), pImage->height() };
				ImgRect DstRect = { Spread, Spread, pImage->width(), pImage->height() };
				
				pPaddedImage->create( NewWidth, NewHeight, &FillColour );
				pPaddedImage->blit( pImage, SrcRect, DstRect );
								
				// Distance field.
				ImgImage* pDistanceFieldImage = pPaddedImage->generateDistanceField( 128, (BcReal)Spread );
								
				// Scale down 8x.
				ImgImage* pScale1_2 = pDistanceFieldImage->resize( NewWidth >> 1, NewHeight >> 1 );
				ImgImage* pScale1_4 = pScale1_2->resize( NewWidth >> 2, NewHeight >> 2 );			
							
				// Crop to final size.
				ImgImage* pFinal = pScale1_4->canvasSize( ( ( pImage->width() + SpreadDouble ) / 4 ), ( ( pImage->height() + SpreadDouble ) / 4 ), &FillColour );

				//
				delete pPaddedImage;
				delete pDistanceFieldImage;
				delete pScale1_2;
				delete pScale1_4;
				delete pImage;
				
				pImage = pFinal;
			}
			
			// Add to list (even if null).
			ImageList.push_back( pImage );
		}

		// If we have images, generate an atlas and export.
		if( ImageList.size() > 0 )
		{
			BcStream HeaderStream;
			BcStream RectsStream;
						
			// Create an atlas of all source textures..
			ImgRectList RectList;
			ImgImage* pAtlasImage = ImgImage::generateAtlas( ImageList, RectList, 256, 256, ClearColour );
			
			// Setup header.
			ScnTextureAtlasHeader Header = 
			{
				ImageList.size()
			};
			
			HeaderStream << Header;
						
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				ImgRect& Rect = RectList[ Idx ];
				ScnTextureAtlasRect OutRect = 
				{
					{
						BcReal( Rect.X_ + ( Spread / 4 ) ) / BcReal( pAtlasImage->width() ),
						BcReal( Rect.Y_ + ( Spread / 4 ) ) / BcReal( pAtlasImage->height() ),
						BcReal( Rect.W_ - ( SpreadDouble / 4 ) ) / BcReal( pAtlasImage->width() ),
						BcReal( Rect.H_ - ( SpreadDouble / 4 ) ) / BcReal( pAtlasImage->height() )
					}
				};
				
				RectsStream << OutRect;
			}
			
			// Add chunks.
			Importer.addChunk( BcHash( "atlasheader" ), HeaderStream.pData(), HeaderStream.dataSize() );
			Importer.addChunk( BcHash( "atlasrects" ), RectsStream.pData(), RectsStream.dataSize() );

			// NOTE: Need a better solution for this. Don't want to reimport this texture.
			CsDependencyList TextureDependancyList;

			// Create a texture.
			std::string AtlasName = Object[ "name" ].asString() + "textureatlas";
			std::string AtlasFileName = std::string( "IntermediateContent/" ) + AtlasName + ".png";
			Img::save( AtlasFileName.c_str(), pAtlasImage );
			
			// Delete all images.
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				delete ImageList[ Idx ];
			}
			ImageList.clear();
			delete pAtlasImage;
			
			// Setup base object, and import.
			Json::Value BaseObject = Object;
			BaseObject[ "source" ] = AtlasFileName;
			
			// Import base texture.
			return Super::import( Importer, BaseObject );
		}
	}
	
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnTextureAtlas );

//////////////////////////////////////////////////////////////////////////
// getRect
//virtual
const ScnRect& ScnTextureAtlas::getRect( BcU32 Idx )
{
	if( Idx < pAtlasHeader_->NoofTextures_ )
	{
		return pAtlasRects_[ Idx ].Rect_;
	}

	return ScnTexture::getRect( Idx );
}

//////////////////////////////////////////////////////////////////////////
// noofRects
//virtual
BcU32 ScnTextureAtlas::noofRects()
{
	return pAtlasHeader_->NoofTextures_;
}


//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void ScnTextureAtlas::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "atlasheader" ) )
	{
		pAtlasHeader_ = (ScnTextureAtlasHeader*)pData;
		
		requestChunk( ++ChunkIdx );
	}
	else if( ChunkID == BcHash( "atlasrects" ) )
	{
		pAtlasRects_ = (ScnTextureAtlasRect*)pData;
		
		requestChunk( ++ChunkIdx, &Header_ );
	}
	else
	{
		Super::fileChunkReady( ChunkIdx, ChunkID, pData );
	}	
}


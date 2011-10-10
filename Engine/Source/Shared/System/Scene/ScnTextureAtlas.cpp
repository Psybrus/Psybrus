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

#include "ScnTextureAtlas.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#include "Img.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnTextureAtlas::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& Source = Object[ "source" ];
	const Json::Value& DistanceFieldValue = Object[ "distancefield" ];
	const Json::Value& SpreadValue = Object[ "spread" ];
	const Json::Value& AlphaFromIntensityValue = Object[ "alphafromintensity" ];
	if( Source.isArray() )
	{
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
			DependancyList.push_back( FileName );

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
				
				pPaddedImage->create( NewWidth, NewHeight, imgFMT_RGBA, &FillColour );
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
			ImgImage* pAtlasImage = ImgImage::generateAtlas( ImageList, RectList, 1024, 1024 );
			
			// Setup header.
			TAtlasHeader Header = 
			{
				ImageList.size()
			};
			
			HeaderStream << Header;
						
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				ImgRect& Rect = RectList[ Idx ];
				ScnRect OutRect = 
				{
					BcReal( Rect.X_ + ( Spread / 4 ) ) / BcReal( pAtlasImage->width() ),
					BcReal( Rect.Y_ + ( Spread / 4 ) ) / BcReal( pAtlasImage->height() ),
					BcReal( Rect.W_ - ( SpreadDouble / 4 ) ) / BcReal( pAtlasImage->width() ),
					BcReal( Rect.H_ - ( SpreadDouble / 4 ) ) / BcReal( pAtlasImage->height() )
				};
				
				RectsStream << OutRect;
			}
			
			// Add chunks.
			pFile_->addChunk( BcHash( "atlasheader" ), HeaderStream.pData(), HeaderStream.dataSize() );
			pFile_->addChunk( BcHash( "atlasrects" ), RectsStream.pData(), RectsStream.dataSize() );

			// NOTE: Need a better solution for this. Don't want to reimport this texture.
			CsDependancyList TextureDependancyList;

			// Create a texture.
			std::string AtlasName = Object[ "name" ].asString() + "_texture_atlas";
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
			return Super::import( BaseObject, TextureDependancyList );
		}
	}
		
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnTextureAtlas );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnTextureAtlas::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	PropertyTable.begin()
		.field( "source",					csPVT_FILE,			csPCT_LIST )
		.field( "distancefield",			csPVT_BOOL,			csPCT_VALUE )
		.field( "spread",					csPVT_UINT,			csPCT_VALUE )
		.field( "alphafromintensity",		csPVT_BOOL,			csPCT_VALUE )
	.end();
}

//////////////////////////////////////////////////////////////////////////
// getRect
//virtual
const ScnRect& ScnTextureAtlas::getRect( BcU32 Idx )
{
	if( Idx < pAtlasHeader_->NoofTextures_ )
	{
		return pAtlasRects_[ Idx ];
	}

	return ScnTexture::getRect( Idx );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void ScnTextureAtlas::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "atlasheader" ) )
	{
		pAtlasHeader_ = (TAtlasHeader*)pData;
		
		getChunk( ++ChunkIdx );
	}
	else if( pChunk->ID_ == BcHash( "atlasrects" ) )
	{
		pAtlasRects_ = (ScnRect*)pData;
		
		getChunk( ++ChunkIdx );
	}
	else
	{
		Super::fileChunkReady( ChunkIdx, pChunk, pData );
	}	
}


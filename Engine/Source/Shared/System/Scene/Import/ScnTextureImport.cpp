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
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnTextureImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Format_", &ScnTextureImport::Format_, bcRFF_IMPORTER ),
		new ReField( "ClearColour_", &ScnTextureImport::ClearColour_, bcRFF_IMPORTER ),
		new ReField( "AlphaFromIntensity_", &ScnTextureImport::AlphaFromIntensity_, bcRFF_IMPORTER ),
		new ReField( "DistanceField_", &ScnTextureImport::DistanceField_, bcRFF_IMPORTER ),
		new ReField( "Spread_", &ScnTextureImport::Spread_, bcRFF_IMPORTER ),
		new ReField( "Type_", &ScnTextureImport::Type_, bcRFF_IMPORTER ),
		new ReField( "Width_", &ScnTextureImport::Width_, bcRFF_IMPORTER ),
		new ReField( "Height_", &ScnTextureImport::Height_, bcRFF_IMPORTER ),
		new ReField( "Depth_", &ScnTextureImport::Depth_, bcRFF_IMPORTER ),
		new ReField( "Levels_", &ScnTextureImport::Levels_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnTextureImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTextureImport::ScnTextureImport():
	Source_(),
	Format_( RsTextureFormat::R8G8B8A8 ),
	ClearColour_( 0.0f, 0.0f, 0.0f, 0.0f ),
	AlphaFromIntensity_( BcFalse ),
	DistanceField_( BcFalse ),
	Spread_( 0 ),
	Type_( RsTextureType::UNKNOWN ),
	Width_( 0 ),
	Height_( 0 ),
	Depth_( 0 ),
	Levels_( 1 )
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
		const Json::Value& )
{
	ImgColour ClearColour = { 0, 0, 0, 0 };
	ClearColour.R_ = BcU8( BcClamp( BcU32( ClearColour_.r() * 255.0f ), 0, 255 ) );
	ClearColour.G_ = BcU8( BcClamp( BcU32( ClearColour_.g() * 255.0f ), 0, 255 ) );
	ClearColour.B_ = BcU8( BcClamp( BcU32( ClearColour_.b() * 255.0f ), 0, 255 ) );
	ClearColour.A_ = BcU8( BcClamp( BcU32( ClearColour_.a() * 255.0f ), 0, 255 ) );
	BcU32 SpreadDouble = Spread_ * 2;

	// Texture atlas.
	if( Source_.size() > 1 )
	{
		// Load all source images.
		ImgImageList ImageList;
		
		for( BcU32 Idx = 0; Idx < Source_.size(); ++Idx )
		{
			const std::string& FileName = Source_[ Idx ];  

			// Add as dependancy.
			CsResourceImporter::addDependency( FileName.c_str() );

			// Load image.
			ImgImageUPtr pImage = Img::load( FileName.c_str() );

			// Generate alpha from intensity.
			if( pImage != NULL && AlphaFromIntensity_ )
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
			if( pImage != NULL && DistanceField_ == BcTrue )
			{
				ImgImageUPtr pPaddedImage = ImgImageUPtr( new ImgImage() );

				ImgColour FillColour = { 0, 0, 0, 0 };
				BcU32 NewWidth = BcPotNext( pImage->width() + SpreadDouble );
				BcU32 NewHeight = BcPotNext( pImage->height() + SpreadDouble );
				
				ImgRect SrcRect = { 0, 0, pImage->width(), pImage->height() };
				ImgRect DstRect = { Spread_, Spread_, pImage->width(), pImage->height() };
				
				pPaddedImage->create( NewWidth, NewHeight, &FillColour );
				pPaddedImage->blit( pImage.get(), SrcRect, DstRect );
								
				// Distance field.
				ImgImageUPtr pDistanceFieldImage = pPaddedImage->generateDistanceField( 128, (BcF32)Spread_ );
								
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
						BcF32( Rect.X_ + ( Spread_ / 4 ) ) / BcF32( pAtlasImage->width() ),
						BcF32( Rect.Y_ + ( Spread_ / 4 ) ) / BcF32( pAtlasImage->height() ),
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
			std::string AtlasName = Name_ + "textureatlas";
			std::string AtlasFileName = getIntermediatePath() + std::string( "/" ) + AtlasName + ".png";
			Img::save( AtlasFileName.c_str(), pAtlasImage.get() );
			
			ImageList.clear();
			pAtlasImage = nullptr;

			// Fall through to the next section.
			// TODO: Refactor so we don't do it this awful way.
			Source_.clear();
			Source_.push_back( AtlasFileName );

			// Don't do distance field when falling through.
			DistanceField_ = BcFalse;
		}
	}

	if( Source_.size() == 1 )
	{
		const std::string& FileName = Source_[ 0 ];

		// Add root dependency.
		CsResourceImporter::addDependency( FileName.c_str() );

		// Load texture from file and create the data for export.
		ImgImageList MipImages;
		MipImages.push_back( Img::load( FileName.c_str() ) );

		// Replace with a distance field version.
		if( MipImages[ 0 ] != nullptr && DistanceField_ == BcTrue )
		{
			ImgImageUPtr pPaddedImage = ImgImageUPtr( new ImgImage() );

			ImgColour FillColour = { 0, 0, 0, 0 };
			BcU32 NewWidth = BcPotNext( MipImages[ 0 ]->width() + SpreadDouble );
			BcU32 NewHeight = BcPotNext( MipImages[ 0 ]->height() + SpreadDouble );
				
			ImgRect SrcRect = { 0, 0, MipImages[ 0 ]->width(), MipImages[ 0 ]->height() };
			ImgRect DstRect = { Spread_, Spread_, MipImages[ 0 ]->width(), MipImages[ 0 ]->height() };
				
			pPaddedImage->create( NewWidth, NewHeight, &FillColour );
			pPaddedImage->blit( MipImages[ 0 ].get(), SrcRect, DstRect );
								
			// Distance field.
			ImgImageUPtr pDistanceFieldImage = pPaddedImage->generateDistanceField( 128, (BcF32)Spread_ );
								
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
		ScnTextureHeader Header = 
		{ 
			Width_, Height_, Depth_, Levels_,
			Type_, RsTextureFormat::R8G8B8A8, BcTrue 
		};
		BcStream HeaderStream;
		HeaderStream << Header;
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
		return BcTrue;
	}

	return BcFalse;
}

#endif
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

#if PSY_IMPORT_PIPELINE

#include "System/Scene/Rendering/ScnTextureFileData.h"
#include "System/Scene/Rendering/ScnTextureAtlasFileData.h"

#include "System/Renderer/RsTypes.h"

#include "Base/BcFile.h"
#include "Base/BcStream.h"

#endif // PSY_IMPORT_PIPELINE

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnTextureImport )
	
void ScnTextureImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnTextureImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Format_", &ScnTextureImport::Format_, bcRFF_IMPORTER ),
		new ReField( "RenderTarget_", &ScnTextureImport::RenderTarget_, bcRFF_IMPORTER ),
		new ReField( "DepthStencilTarget_", &ScnTextureImport::DepthStencilTarget_, bcRFF_IMPORTER ),
		new ReField( "ClearColour_", &ScnTextureImport::ClearColour_, bcRFF_IMPORTER ),
		new ReField( "AlphaFromIntensity_", &ScnTextureImport::AlphaFromIntensity_, bcRFF_IMPORTER ),
		new ReField( "DistanceField_", &ScnTextureImport::DistanceField_, bcRFF_IMPORTER ),
		new ReField( "Spread_", &ScnTextureImport::Spread_, bcRFF_IMPORTER ),
		new ReField( "Spread_", &ScnTextureImport::Spread_, bcRFF_IMPORTER ),
		new ReField( "TileAtlas_", &ScnTextureImport::TileAtlas_, bcRFF_IMPORTER ),
		new ReField( "TileWidth_", &ScnTextureImport::TileWidth_, bcRFF_IMPORTER ),
		new ReField( "TileHeight_", &ScnTextureImport::TileHeight_, bcRFF_IMPORTER ),
		new ReField( "RoundUpPowerOfTwo_", &ScnTextureImport::RoundUpPowerOfTwo_, bcRFF_IMPORTER ),
		new ReField( "RoundDownPowerOfTwo_", &ScnTextureImport::RoundDownPowerOfTwo_, bcRFF_IMPORTER ),
		new ReField( "TextureType_", &ScnTextureImport::TextureType_, bcRFF_IMPORTER ),
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
	Format_( RsTextureFormat::UNKNOWN ),
	RenderTarget_( BcFalse ),
	DepthStencilTarget_( BcFalse ),
	ClearColour_( 0.0f, 0.0f, 0.0f, 0.0f ),
	AlphaFromIntensity_( BcFalse ),
	DistanceField_( BcFalse ),
	Spread_( 0 ),
	Border_( 0 ),
	TileAtlas_( BcFalse ),
	TileWidth_( 0 ),
	TileHeight_( 0 ),
	RoundUpPowerOfTwo_( BcFalse ),
	RoundDownPowerOfTwo_( BcFalse ),
	TextureType_( RsTextureType::UNKNOWN ),
	Width_( 0 ),
	Height_( 0 ),
	Depth_( 0 ),
	Levels_( 1 )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTextureImport::ScnTextureImport( ReNoInit ):
	Source_(),
	Format_( RsTextureFormat::UNKNOWN ),
	RenderTarget_( BcFalse ),
	DepthStencilTarget_( BcFalse ),
	ClearColour_( 0.0f, 0.0f, 0.0f, 0.0f ),
	AlphaFromIntensity_( BcFalse ),
	DistanceField_( BcFalse ),
	Spread_( 0 ),
	Border_( 0 ),
	TileAtlas_( BcFalse ),
	TileWidth_( 0 ),
	TileHeight_( 0 ),
	RoundUpPowerOfTwo_( BcFalse ),
	RoundDownPowerOfTwo_( BcFalse ),
	TextureType_( RsTextureType::UNKNOWN ),
	Width_( 0 ),
	Height_( 0 ),
	Depth_( 0 ),
	Levels_( 1 )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTextureImport::ScnTextureImport( 
		const std::string Name,
		const std::string Type,
		const std::string Source,
		RsTextureFormat Format,
		BcU32 TileWidth,
		BcU32 TileHeight ):
	CsResourceImporter( Name, Type ),
	Format_( Format ),
	RenderTarget_( BcFalse ),
	DepthStencilTarget_( BcFalse ),
	ClearColour_( 0.0f, 0.0f, 0.0f, 0.0f ),
	AlphaFromIntensity_( BcFalse ),
	DistanceField_( BcFalse ),
	Spread_( 0 ),
	Border_( 0 ),
	TileAtlas_( BcTrue ),
	TileWidth_( TileWidth ),
	TileHeight_( TileHeight ),
	RoundUpPowerOfTwo_( BcFalse ),
	RoundDownPowerOfTwo_( BcFalse ),
	TextureType_( RsTextureType::UNKNOWN ),
	Width_( 0 ),
	Height_( 0 ),
	Depth_( 0 ),
	Levels_( 1 )
{
	Source_.push_back( Source );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTextureImport::ScnTextureImport( 
		const std::string Name,
		const std::string Type,
		const std::string Source,
		RsTextureFormat Format ):
	CsResourceImporter( Name, Type ),
	Format_( Format ),
	RenderTarget_( BcFalse ),
	DepthStencilTarget_( BcFalse ),
	ClearColour_( 0.0f, 0.0f, 0.0f, 0.0f ),
	AlphaFromIntensity_( BcFalse ),
	DistanceField_( BcFalse ),
	Spread_( 0 ),
	Border_( 0 ),
	TileAtlas_( BcFalse ),
	TileWidth_( 0 ),
	TileHeight_( 0 ),
	RoundUpPowerOfTwo_( BcFalse ),
	RoundDownPowerOfTwo_( BcFalse ),
	TextureType_( RsTextureType::UNKNOWN ),
	Width_( 0 ),
	Height_( 0 ),
	Depth_( 0 ),
	Levels_( 1 )
{
	Source_.push_back( Source );
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
#if PSY_IMPORT_PIPELINE
	ImgColour ClearColour = { 0, 0, 0, 0 };
	ClearColour.R_ = BcU8( BcClamp( BcU32( ClearColour_.r() * 255.0f ), 0, 255 ) );
	ClearColour.G_ = BcU8( BcClamp( BcU32( ClearColour_.g() * 255.0f ), 0, 255 ) );
	ClearColour.B_ = BcU8( BcClamp( BcU32( ClearColour_.b() * 255.0f ), 0, 255 ) );
	ClearColour.A_ = BcU8( BcClamp( BcU32( ClearColour_.a() * 255.0f ), 0, 255 ) );
	BcU32 SpreadDouble = Spread_ * 2;
	if( Source_.size() > 0 )
	{
		// Load all source images.
		ImgImageList ImageList = loadImages( Source_ );
		
		if( ImageList.size() == 0 )
		{
			return BcFalse;
		}

		// If a tile atlas, reconstruct the image list.
		if( TileAtlas_ )
		{
			if( getResourceType() != "ScnTextureAtlas" )
			{
				CsResourceImporter::addMessage( CsMessageCategory::ERROR, "Need to set resource type to ScnTextureAtlas." );
				return BcFalse;

			}
			ImgImageList NewImageList;
			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				auto& Image = ImageList[ Idx ];
				for( BcU32 Y = 0; Y <= ( Image->height() - TileHeight_ ); Y += TileHeight_ )
				{
					for( BcU32 X = 0; X <= ( Image->width() - TileWidth_ ); X += TileWidth_ )
					{
						ImgRect OutRect = 
						{
							X,
							Y,
							TileWidth_,
							TileHeight_
						};

						auto NewImage = Image->get( OutRect );
						NewImageList.push_back( std::move( NewImage ) );
					}
				}
			}

			ImageList = std::move( NewImageList );
		}

		// Iterate over all images, process appropriately.
		for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
		{
			auto& Image = ImageList[ Idx ];

			// Generate alpha from intensity.
			if( Image != nullptr && AlphaFromIntensity_ )
			{
				Image = processAlphaFromIntensity( std::move( Image ) );
			}

			// Replace with a distance field version.
			if( Image != nullptr && DistanceField_ == BcTrue )
			{
				Image = processDistanceField( std::move( Image ), Spread_ );
			}
		}

		// If we have images, generate an atlas and export.
		if( ImageList.size() > 1 )
		{					
			BcStream AtlasHeaderStream;
			BcStream AtlasRectsStream;

			// Create an atlas of all source textures..
			ImgRectList RectList;
			ImgImageUPtr AtlasImage = ImgImage::generateAtlas( ImageList, RectList, 64, 64, ClearColour );
						
			// Setup header.
			ScnTextureAtlasHeader Header = 
			{
				(BcU32)ImageList.size()
			};

			AtlasHeaderStream << Header;

			for( BcU32 Idx = 0; Idx < ImageList.size(); ++Idx )
			{
				ImgRect& Rect = RectList[ Idx ];
				ScnTextureAtlasRect OutRect = 
				{
					{
						BcF32( Rect.X_ ) / BcF32( AtlasImage->width() ),
						BcF32( Rect.Y_ ) / BcF32( AtlasImage->height() ),
						BcF32( Rect.W_ ) / BcF32( AtlasImage->width() ),
						BcF32( Rect.H_ ) / BcF32( AtlasImage->height() )
					}
				};
				
				AtlasRectsStream << OutRect;
			}
			
			ImageList.clear();
			ImageList.push_back( std::move( AtlasImage ) );

			CsResourceImporter::addChunk( BcHash( "atlasheader" ), AtlasHeaderStream.pData(), AtlasHeaderStream.dataSize() );
			CsResourceImporter::addChunk( BcHash( "atlasrects" ), AtlasRectsStream.pData(), AtlasRectsStream.dataSize() );
		}

		// Should only have 1 image at this point.
		BcAssert( ImageList.size() == 1 );
		auto Image = std::move( ImageList[ 0 ] );

		// Do to power of two or something.
		if( RoundUpPowerOfTwo_ )
		{
			Image = processRoundUpPot( std::move( Image ) );
		}

		if( RoundDownPowerOfTwo_ )
		{
			Image = processRoundDownPot( std::move( Image ) );
		}

		// Downsample texture for mip maps if POT.
		auto MipImages = generateMipMaps( std::move( Image ) );

		// Encode the image as a format.
		BcU8* pEncodedImageData = NULL;
		BcU32 EncodedImageDataSize = 0;

		// Check if type is unknown.
		if( TextureType_ == RsTextureType::UNKNOWN )
		{
			TextureType_ = MipImages[ 0 ]->height() == 1 ? RsTextureType::TEX1D : RsTextureType::TEX2D;
		}

		if( TextureType_ == RsTextureType::TEXCUBE )
		{
			if( MipImages[ 0 ]->width() != ( MipImages[ 0 ]->height() * 6 ) )
			{
				CsResourceImporter::addMessage( CsMessageCategory::ERROR, "TextureType is cube map, but dimensions of texture are not correct (6x horizontal images)." );
				return BcFalse;
			}

			if( BcPot( MipImages[ 0 ]->height() ) == BcFalse )
			{
				CsResourceImporter::addMessage( CsMessageCategory::ERROR, "TextureType is cube map, but each face is not a power of two width + height." );
				return BcFalse;
			}
		}

		// Automatically determine the best format if we specify unknown.
		if( Format_ == RsTextureFormat::UNKNOWN || Format_ == RsTextureFormat::INVALID )
		{
			// Default to a catch all which is 32 bit RGBA.
			Format_ = RsTextureFormat::R8G8B8A8;

			// In a non-debug build, check if we should
			// use texture compression (to speed up build times).
			if( TextureType_ == RsTextureType::TEX2D )
			{
				if( MipImages[ 0 ]->width() % 4 == 0 && 
					MipImages[ 0 ]->height() % 4 == 0 )
				{
					if( MipImages[ 0 ]->hasAlpha( 8 ) == BcFalse )
					{
						Format_ = RsTextureFormat::DXT1;
					}
					else
					{
						Format_ = RsTextureFormat::DXT5;
					}
				}
			}
		}

		// Streams.
		BcStream HeaderStream;
		BcStream BodyStream( BcFalse, 1024, EncodedImageDataSize );

		// Write all mip images into the same body for now.
		for( BcU32 Idx = 0; Idx < MipImages.size(); ++Idx )
		{
			auto* pImage = MipImages[ Idx ].get();
			for( BcU32 Attempt = 0; Attempt < 2; ++Attempt )
			{
				ImgEncodeFormat EncodeFormat = (ImgEncodeFormat)Format_;
				if( pImage->encodeAs( EncodeFormat, pEncodedImageData, EncodedImageDataSize ) )
				{
					// Serialize encoded images.
					BodyStream.push( pEncodedImageData, EncodedImageDataSize );
					delete [] pEncodedImageData;
					pEncodedImageData = NULL;
					EncodedImageDataSize = 0;
					Attempt = 2;
				}
				else
				{
					PSY_LOG( "Failed to encode image, falling back to R8G8B8A8\n" );
					Format_ = RsTextureFormat::R8G8B8A8;
				}
			}
		}

		// Write header.
		ScnTextureHeader Header =
		{
			static_cast< BcS32 >( MipImages[ 0 ]->width() ) / ( TextureType_ == RsTextureType::TEXCUBE ? 6 : 1 ),
			static_cast< BcS32 >( MipImages[ 0 ]->height() ),
			0,
			(BcU32)MipImages.size(),
			TextureType_,
			Format_,
			BcFalse,
			RenderTarget_,
			DepthStencilTarget_
		};

		HeaderStream << Header;

		// Clear images.
		MipImages.clear();

		// Add chunks.
		BcAssert( BodyStream.dataSize() > 0 );

		// Add chunks.
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
		CsResourceImporter::addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );

		return BcTrue;
	}
	else
	{
		if( TextureType_ == RsTextureType::UNKNOWN )
		{
			CsResourceImporter::addMessage( CsMessageCategory::ERROR, "TextureType is UNKNOWN." );
			return BcFalse;
		}

		// User created texture.
		ScnTextureHeader Header = 
		{ 
			static_cast< BcS32 >( Width_ ), 
			static_cast< BcS32 >( Height_ ), 
			Depth_, 
			Levels_,
			TextureType_, 
			Format_, 
			!RenderTarget_ && !DepthStencilTarget_, // If we're not a render target and not a depth stencil target, we're editable.
			RenderTarget_,
			DepthStencilTarget_
		};
		BcStream HeaderStream;
		HeaderStream << Header;
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
		return BcTrue;
	}

#endif // PSY_IMPORT_PIPELINE
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// loadImages
ImgImageList ScnTextureImport::loadImages( std::vector< std::string > Sources )
{
	ImgImageList Images;

	for( BcU32 Idx = 0; Idx < Source_.size(); ++Idx )
	{
		const std::string& FileName = Source_[ Idx ];  

		// Add as dependancy.
		CsResourceImporter::addDependency( FileName.c_str() );

		// Load image.
		auto Image = Img::load( FileName.c_str() );
		if( Image == nullptr )
		{
			BcChar Error[ 4096 ] = { 0 };
			BcSPrintf( Error, sizeof( Error ) - 1, "Unable to load texture \"%s\"", FileName.c_str() );
			CsResourceImporter::addMessage( CsMessageCategory::ERROR, Error );
		}
		else
		{
			Images.push_back( std::move( Image ) );
		}
	}

	return Images;
}

//////////////////////////////////////////////////////////////////////////
// processAlphaFromIntensity
ImgImageUPtr ScnTextureImport::processAlphaFromIntensity( ImgImageUPtr Image )
{
	for( BcU32 Y = 0; Y < Image->height(); ++Y )
	{
		for( BcU32 X = 0; X < Image->width(); ++X )
		{
			ImgColour Texel = Image->getPixel( X, Y );
			Texel.A_ = (BcU8)( ( (BcU32)Texel.R_ + (BcU32)Texel.G_ + (BcU32)Texel.B_ ) / 3.0f );
			Image->setPixel( X, Y, Texel );
		}
	}

	return std::move( Image );
}

//////////////////////////////////////////////////////////////////////////
// processDistanceField
ImgImageUPtr ScnTextureImport::processDistanceField( ImgImageUPtr Image, BcU32 Spread )
{
	return std::move( Image->generateDistanceField( 128, (BcF32)Spread_ ) );
}

//////////////////////////////////////////////////////////////////////////
// processRoundUpPot
ImgImageUPtr ScnTextureImport::processRoundUpPot( ImgImageUPtr Image )
{
	return Image->resize( 
		BcPotNext( Image->width() ), 
		BcPotNext( Image->height() ),
		1.0f );
}

//////////////////////////////////////////////////////////////////////////
// processRoundDownPot
ImgImageUPtr ScnTextureImport::processRoundDownPot( ImgImageUPtr Image )
{
	BcU32 W = Image->width();
	BcU32 H = Image->height();

	if( !BcPot( W ) || !BcPot( H ) )
	{
		Image = processRoundUpPot( std::move( Image ) );
	}

	return Image->resize( 
		BcPotNext( W ) / 2, 
		BcPotNext( H ) / 2 ,
		1.0f );
}

//////////////////////////////////////////////////////////////////////////
// generateMipMaps
ImgImageList ScnTextureImport::generateMipMaps( ImgImageUPtr Image )
{
	ImgImageList MipImages;
	BcU32 W = Image->width();
	BcU32 H = Image->height();

	MipImages.push_back( std::move( Image ) );

	if( ( BcPot( W ) && BcPot( H ) ) ||
		( ( W / 6 ) == H && BcPot( H ) ) ) // Cubemap.
	{
		while( W > 1 && H > 1 )
		{
			W >>= 1;
			H >>= 1;
			MipImages.push_back( MipImages[ MipImages.size() - 1 ]->resize( W, H, 1.0f ) );
		}
	}
	return MipImages;
}

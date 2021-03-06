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
REFLECTION_DEFINE_DERIVED( ScnTextureImportParams )

void ScnTextureImportParams::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Formats_", &ScnTextureImportParams::Formats_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnTextureImportParams, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTextureImportParams::ScnTextureImportParams()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnTextureImportParams::~ScnTextureImportParams()
{
}

//////////////////////////////////////////////////////////////////////////
// getFormat
RsResourceFormat ScnTextureImportParams::getFormat( const char* FormatName ) const
{
	// Search for format in table.
	if( Formats_.find( FormatName ) != Formats_.end() )
	{
		return Formats_.find( FormatName )->second;
	}

	// Fall back to RsResourceFormat.
	if( auto EnumConstant = ReManager::GetEnum( "RsResourceFormat" )->getEnumConstant( FormatName ) )
	{
		return static_cast< RsResourceFormat >( EnumConstant->getValue() );
	}

	// Can't find, fall back to unknown.
	return RsResourceFormat::UNKNOWN;
}

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
	Format_(),
	EncodeFormat_( ImgEncodeFormat::UNKNOWN ),
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
	Format_(),
	EncodeFormat_( ImgEncodeFormat::UNKNOWN ),
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
		const char* Format,
		BcU32 TileWidth,
		BcU32 TileHeight ):
	CsResourceImporter( Name, Type ),
	Format_( Format ),
	EncodeFormat_( ImgEncodeFormat::UNKNOWN ),
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
		const char* Format,
		ImgEncodeFormat EncodeFormat ):
	CsResourceImporter( Name, Type ),
	Format_( Format ),
	EncodeFormat_( EncodeFormat ),
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
BcBool ScnTextureImport::import()
{
#if PSY_IMPORT_PIPELINE
	ImgColour ClearColour = { 0, 0, 0, 0 };
	ClearColour.R_ = BcU8( BcClamp( BcU32( ClearColour_.r() * 255.0f ), 0, 255 ) );
	ClearColour.G_ = BcU8( BcClamp( BcU32( ClearColour_.g() * 255.0f ), 0, 255 ) );
	ClearColour.B_ = BcU8( BcClamp( BcU32( ClearColour_.b() * 255.0f ), 0, 255 ) );
	ClearColour.A_ = BcU8( BcClamp( BcU32( ClearColour_.a() * 255.0f ), 0, 255 ) );

	// Add type dependencies.
	CsResourceImporter::addDependency( ReManager::GetEnum( "RsResourceFormat" ) );
	CsResourceImporter::addDependency( ReManager::GetEnum( "RsTextureType" ) );

	// Find texture import params.
	auto Params = getImportParams< ScnTextureImportParams > ();
	if( !Params )
	{
		addMessage( CsMessageCategory::ERROR, "Unable to find ScnTextureImportParams in platform config." );
		return BcFalse;
	}

	// Check if it's a DDS, avoid all processing for those.
	if( Source_.size() == 1 && Source_[ 0 ].rfind( ".dds" ) != std::string::npos )
	{
		return loadDDS( Source_[ 0 ].c_str() ) ? BcTrue : BcFalse;
	}

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
		auto ResourceFormat = Params->getFormat( Format_.c_str() );
		if( ResourceFormat == RsResourceFormat::UNKNOWN || ResourceFormat == RsResourceFormat::INVALID )
		{
			if( !Format_.empty() )
			{
				CsResourceImporter::addMessage( CsMessageCategory::ERROR, "Texture format \"%s\" is unknown.", Format_.c_str() );
			}

			// Default to a catch all which is 32 bit RGBA.
			ResourceFormat = Params->getFormat( "R8G8B8A8_UNORM" );
			RsResourceFormat DesiredFormat = ResourceFormat;

			// Grab appropriate format from params.
			if( MipImages[ 0 ]->hasAlpha( 2 ) == BcFalse )
			{
				DesiredFormat = Params->getFormat( "RGB" );
			}
			else
			{
				DesiredFormat = Params->getFormat( "RGBA" );
			}

			auto BlockInfo = RsTextureBlockInfo( ResourceFormat );
			if( MipImages[ 0 ]->width() % BlockInfo.Width_ == 0 && 
				MipImages[ 0 ]->height() % BlockInfo.Height_ == 0 )
			{
				ResourceFormat = DesiredFormat;
			}

			if( Format_.empty() )
			{
				CsResourceImporter::addMessage( CsMessageCategory::WARNING, "Texture format is not specified. Automatically selected \"%s\"",
					(*ReManager::GetEnumValueName( ResourceFormat )).c_str() );
			}
		}

		if( ResourceFormat == RsResourceFormat::UNKNOWN )
		{
			// 
		}

		// Streams.
		BcStream HeaderStream;
		BcStream BodyStream( BcFalse, 1024, EncodedImageDataSize );
		BcBool AllMipsSucceeded = BcTrue;

		// Write all mip images into the same body for now.
		for( BcU32 Attempt = 0; Attempt < 2; ++Attempt )
		{
			BodyStream.clear();
			AllMipsSucceeded = BcTrue;
			for( BcU32 Idx = 0; Idx < MipImages.size(); ++Idx )
			{
				auto* pImage = MipImages[ Idx ].get();
				ImgEncodeFormat EncodeFormat = EncodeFormat_;
				if( EncodeFormat == ImgEncodeFormat::UNKNOWN )
				{
					switch( ResourceFormat ) 
					{
					case RsResourceFormat::R8_UNORM:
					case RsResourceFormat::R8_UINT:
					case RsResourceFormat::R8_SNORM:
					case RsResourceFormat::R8_SINT:
						{
							EncodeFormat = ImgEncodeFormat::I8; 
							if( DistanceField_ || AlphaFromIntensity_ )
							{
								EncodeFormat = ImgEncodeFormat::A8;
							}
						}
						break;

					case RsResourceFormat::R8G8B8A8_UNORM:
					case RsResourceFormat::R8G8B8A8_UNORM_SRGB:
					case RsResourceFormat::R8G8B8A8_UINT:
					case RsResourceFormat::R8G8B8A8_SNORM:
					case RsResourceFormat::R8G8B8A8_SINT:
						EncodeFormat = ImgEncodeFormat::R8G8B8A8; 
						break;
					 
					case RsResourceFormat::BC1_UNORM:
					case RsResourceFormat::BC1_UNORM_SRGB:
						EncodeFormat = ImgEncodeFormat::BC1; 
						break;

					case RsResourceFormat::BC2_UNORM:
					case RsResourceFormat::BC2_UNORM_SRGB:
						EncodeFormat = ImgEncodeFormat::BC2; 
						break;

					case RsResourceFormat::BC3_UNORM:
					case RsResourceFormat::BC3_UNORM_SRGB:
						EncodeFormat = ImgEncodeFormat::BC3; 
						break;

					case RsResourceFormat::BC4_UNORM:
					case RsResourceFormat::BC4_SNORM:
						EncodeFormat = ImgEncodeFormat::BC4; 
						break;

					case RsResourceFormat::BC5_UNORM:
					case RsResourceFormat::BC5_SNORM:
						EncodeFormat = ImgEncodeFormat::BC5; 
						break;

					case RsResourceFormat::ETC1_UNORM:
						EncodeFormat = ImgEncodeFormat::ETC1; 
						break;
					}
				}

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
					PSY_LOG( "Failed to encode image, falling back to R8G8B8A8_UNORM\n" );
					ResourceFormat = RsResourceFormat::R8G8B8A8_UNORM;
					AllMipsSucceeded = BcFalse;
				}
			}


			if( AllMipsSucceeded )
			{
				break;
			}
		}

		if( AllMipsSucceeded == BcFalse )
		{
			PSY_LOG( "ERROR: Failed to encode all image's mip levels." );
			return BcFalse;
		}


		RsBindFlags BindFlags = RsBindFlags::SHADER_RESOURCE;
		if( RenderTarget_ )
		{
			BindFlags |= RsBindFlags::RENDER_TARGET;
		}
		else if ( DepthStencilTarget_ )
		{
			BindFlags |= RsBindFlags::DEPTH_STENCIL;
		}

		// Write header.
		ScnTextureHeader Header =
		{
			static_cast< BcS32 >( MipImages[ 0 ]->width() ) / ( TextureType_ == RsTextureType::TEXCUBE ? 6 : 1 ),
			static_cast< BcS32 >( MipImages[ 0 ]->height() ),
			0,
			(BcU32)MipImages.size(),
			TextureType_,
			ResourceFormat,
			BcFalse,
			BindFlags,
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

		RsBindFlags BindFlags = RsBindFlags::SHADER_RESOURCE;
		if( RenderTarget_ )
		{
			BindFlags |= RsBindFlags::RENDER_TARGET;
		}
		else if ( DepthStencilTarget_ )
		{
			BindFlags |= RsBindFlags::DEPTH_STENCIL;
		}

		auto ResourceFormat = Params->getFormat( Format_.c_str() );

		// User created texture.
		ScnTextureHeader Header = 
		{ 
			static_cast< BcS32 >( Width_ ), 
			static_cast< BcS32 >( Height_ ), 
			Depth_, 
			Levels_,
			TextureType_, 
			ResourceFormat,
			!RenderTarget_ && !DepthStencilTarget_, // If we're not a render target and not a depth stencil target, we're editable.
			BindFlags
		};
		BcStream HeaderStream;
		HeaderStream << Header;
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
		return BcTrue;
	}

#else // PSY_IMPORT_PIPELINE
	return BcFalse;
#endif
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
		auto ResolvedFilename = CsPaths::resolveContent( FileName.c_str() );
		CsResourceImporter::addDependency( ResolvedFilename.c_str() );

		// Load image.
		auto Image = Img::load( ResolvedFilename.c_str() );
		if( Image == nullptr )
		{
			BcChar Error[ 4096 ] = { 0 };
			BcSPrintf( Error, sizeof( Error ) - 1, "Unable to load texture \"%s\"", ResolvedFilename.c_str() );
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
		BcPotNext( Image->height() ), 1.0f );
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
		BcPotNext( H ) / 2, 1.0f );
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

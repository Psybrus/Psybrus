/**************************************************************************
*
* File:		ScnTexture.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnTexture.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_SERVER
#include "Base/BcStream.h"
#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnTexture::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	const Json::Value& Source = Object[ "source" ];
	const Json::Value& Format = Object[ "format" ];

	if( Source.type() == Json::stringValue )
	{
		const std::string& FileName = Object[ "source" ].asString();

		// Add root dependency.
		Importer.addDependency( FileName.c_str() );

		// Load texture from file and create the data for export.
		ImgImage* pImage = Img::load( FileName.c_str() );
		ImgImage* pTopLevelImage = pImage;

		if( pImage != NULL )
		{
			std::vector< ImgImage* > MipImages;
			BcU32 W = pImage->width();
			BcU32 H = pImage->height();

			// Push main image in first.
			MipImages.push_back( pImage );
		
			// Downsample texture for mip maps.
			if( BcPot( W ) && BcPot( H ) )
			{
				// Down to a minimum of 4x4.
				while( W > 4 && H > 4 )
				{
					W >>= 1;
					H >>= 1;

					pImage = pImage->resize( W, H );
					MipImages.push_back( pImage );
				}
			}

			// Encode the image as a format.
			BcU8* pEncodedImageData = NULL;
			BcU32 EncodedImageDataSize = 0;

			// TODO: Take from parameters.
			ImgEncodeFormat EncodeFormat = imgEF_RGBA8;
			RsTextureFormat TextureFormat = RsTextureFormat::R8G8B8A8;
			RsTextureType TextureType = pTopLevelImage->height() == 0 ? RsTextureType::TEX1D : RsTextureType::TEX2D;
		
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
				pTopLevelImage->width(), 
				pTopLevelImage->height(), 
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
				pImage = MipImages[ Idx ];
				if( pImage->encodeAs( EncodeFormat, pEncodedImageData, EncodedImageDataSize ) )
				{
					// Serialize encoded images.
					BodyStream.push( pEncodedImageData, EncodedImageDataSize );
					delete [] pEncodedImageData;
					pEncodedImageData = NULL;
					EncodedImageDataSize = 0;
							
					// Delete image.
					delete pImage;
				}
				else
				{
					BcPrintf( "Failed to encode image \"%s\"\n", FileName.c_str() );
				}
			}

			// Clear images.
			MipImages.clear();

			// Add chunks.
			Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
			Importer.addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );

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
		Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
		return BcTrue;
	}

	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnTexture );

void ScnTexture::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pTexture_",			&ScnTexture::pTexture_,			bcRFF_TRANSIENT ),
		new ReField( "Header_",				&ScnTexture::Header_ ),
	};
		
	ReRegisterClass< ScnTexture, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise()
{
	// NULL internals.
	pTexture_ = NULL;
	pTextureData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise( BcU32 Width, BcU32 Levels, RsTextureFormat Format )
{
	Super::initialise();

	// NULL internals.
	pTexture_ = NULL;
	pTextureData_ = NULL;

	Header_.Width_ = Width;
	Header_.Height_ = 0;
	Header_.Depth_ = 0;
	Header_.Levels_ = Levels;
	Header_.Type_ = RsTextureType::TEX1D;
	Header_.Format_ = Format;
	Header_.Editable_ = BcTrue;

	markCreate();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format )
{
	Super::initialise();

	// NULL internals.
	pTexture_ = NULL;
	pTextureData_ = NULL;

	Header_.Width_ = Width;
	Header_.Height_ = Height;
	Header_.Depth_ = 0;
	Header_.Levels_ = Levels;
	Header_.Type_ = RsTextureType::TEX2D;
	Header_.Format_ = Format;
	Header_.Editable_ = BcTrue;

	markCreate();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, RsTextureFormat Format )
{
	Super::initialise();

	// NULL internals.
	pTexture_ = NULL;
	pTextureData_ = NULL;

	Header_.Width_ = Width;
	Header_.Height_ = Height;
	Header_.Depth_ = Depth;
	Header_.Levels_ = Levels;
	Header_.Type_ = RsTextureType::TEX3D;
	Header_.Format_ = Format;
	Header_.Editable_ = BcTrue;

	markCreate();
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnTexture::create()
{
	if( Header_.Editable_ )
	{
		// Allocate to a 4k alignment.
		pTextureData_ = BcMemAlign(
			RsTextureFormatSize( 
				Header_.Format_, 
				Header_.Width_, 
				Header_.Height_,
				Header_.Depth_, 
				Header_.Levels_ ), 4096 );
	}

	// Create new one immediately.
	pTexture_ = RsCore::pImpl()->createTexture( 
		RsTextureDesc( 
			Header_.Type_, 
			RsResourceCreationFlags::STATIC,
			Header_.Format_,
			Header_.Levels_,
			Header_.Width_,
			Header_.Height_,
			Header_.Depth_ ) );

	// Upload texture data.
	BcU8* TextureData = reinterpret_cast< BcU8* >( pTextureData_ );

	BcU32 Width = Header_.Width_;
	BcU32 Height = Header_.Height_;
	BcU32 Depth = Header_.Depth_;
	for( BcU32 LevelIdx = 0; LevelIdx < Header_.Levels_; ++LevelIdx )
	{
		auto Slice = pTexture_->getSlice( LevelIdx );

		BcU32 SliceSize = 
			RsTextureFormatSize( 
				Header_.Format_, 
				Width, 
				Height,
				Depth, 
				1 );

		RsCore::pImpl()->updateTexture( 
			pTexture_,
			Slice,
			RsResourceUpdateFlags::ASYNC,
			[ TextureData, SliceSize ]( RsTexture* Texture, const RsTextureLock& Lock )
			{
				if( Lock.Buffer_ != nullptr )
				{
					BcMemCopy( Lock.Buffer_, TextureData, SliceSize );
				}
			} );

		// Down a level.
		Width = BcMax( 1, Width >> 1 );
		Height = BcMax( 1, Height >> 1 );
		Depth = BcMax( 1, Depth >> 1 );

		// Advance texture data.
		TextureData += SliceSize;
	}


	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnTexture::destroy()
{
	RsCore::pImpl()->destroyResource( pTexture_ );
	pTexture_ = NULL;

	// Free if it's user created.
	if( Header_.Editable_ )
	{
		BcMemFree( pTextureData_ );
		pTextureData_ = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// getTexture
RsTexture* ScnTexture::getTexture()
{
	return pTexture_;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
BcU32 ScnTexture::getWidth() const
{
	return Header_.Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
BcU32 ScnTexture::getHeight() const
{
	return Header_.Height_;
}

//////////////////////////////////////////////////////////////////////////
// getRect
//virtual
const ScnRect& ScnTexture::getRect( BcU32 Idx )
{
	static ScnRect Rect = 
	{
		0.0f, 0.0f,
		1.0f, 1.0f
	};
	
	return Rect;
}

//////////////////////////////////////////////////////////////////////////
// noofRects
//virtual
BcU32 ScnTexture::noofRects()
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnTexture::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0, &Header_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnTexture::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		requestChunk( 0 );
		return;
	}

	if( ChunkID == BcHash( "header" ) )
	{		
		/*
		// Request all texture levels.
		for( BcU32 iLevel = 0; iLevel < Header_.Levels_; ++iLevel )
		{
			requestChunk( ++ChunkIdx );
		}
		*/

		if( Header_.Editable_ == BcFalse )
		{
			requestChunk( ++ChunkIdx );
		}
		else
		{
			markCreate();
		}
	}
	else if( ChunkID == BcHash( "body" ) )
	{
		// Grab pointer to data.
		BcAssert( pTextureData_ == NULL || pTextureData_ == pData );
		pTextureData_ = pData;
		
		markCreate();
	}
}


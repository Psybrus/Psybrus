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

#ifdef PSY_SERVER
#include "Base/BcStream.h"
#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnTexture::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	const std::string& FileName = Object[ "source" ].asString();

	// Add root dependency.
	Importer.addDependency( FileName.c_str() );

	// Load texture from file and create the data for export.
	ImgImage* pImage = Img::load( FileName.c_str() );
	
	if( pImage != NULL )
	{
		// Encode the image as a format.
		BcU8* pEncodedImageData = NULL;
		BcU32 EncodedImageDataSize = 0;

		// TODO: Take from parameters.
		ImgEncodeFormat EncodeFormat = imgEF_RGBA8;
		eRsTextureFormat TextureFormat = rsTF_RGBA8;

		if( pImage->encodeAs( EncodeFormat, pEncodedImageData, EncodedImageDataSize ) )
		{
			// Serialize encoded image.
			BcStream BodyStream( BcFalse, 1024, EncodedImageDataSize );
			BodyStream.push( pEncodedImageData, EncodedImageDataSize );
			delete pEncodedImageData;
			pEncodedImageData = NULL;


			BcStream HeaderStream;
			THeader Header = { pImage->width(), pImage->height(), 1, TextureFormat };
			HeaderStream << Header;
			
			// Delete image.
			delete pImage;
			
			// Add chunks and finish up.
			Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
			Importer.addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );
			
			//
			return BcTrue;
		}
		else
		{
			BcPrintf( "Failed to encode image \"%s\"\n", FileName.c_str() );
		}
	}
	else
	{
		BcPrintf( "Failed to load image \"%s\"\n", FileName.c_str() );
	}
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnTexture );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise()
{
	// NULL internals.
	pTexture_ = NULL;
	pTextureData_ = NULL;
	CreateNewTexture_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format )
{
	// NULL internals.
	pTexture_ = NULL;
	pTextureData_ = NULL;
	CreateNewTexture_ = BcTrue;

	Header_.Width_ = Width;
	Header_.Height_ = Height;
	Header_.Levels_ = Levels;
	Header_.Format_ = Format; // BAD.

	pTextureData_ = new BcU32[ Header_.Width_ * Header_.Height_ * Header_.Levels_ ];

	setup();
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnTexture::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnTexture::destroy()
{
	RsCore::pImpl()->destroyResource( pTexture_ );
	pTexture_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnTexture::isReady()
{
	// We're ready if we have a valid texture pointer.
	return pTexture_ != NULL && pTexture_->hasHandle();
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
// getTexel
RsColour ScnTexture::getTexel( BcU32 X, BcU32 Y ) const
{
	if( pTextureData_ != NULL && X < Header_.Width_ && Y < Header_.Height_ )
	{
		BcU32* pTextureData = (BcU32*)pTextureData_;
		BcU32 Index = X + Y * Header_.Width_;
		BcU32 Texel = pTextureData[ Index ] ;
		return RsColour( Texel ); // invalid. need to fix.
	}
	
	return RsColour( 0.0f, 0.0f, 0.0f, 0.0f );
}

//////////////////////////////////////////////////////////////////////////
// setTexel
void ScnTexture::setTexel( BcU32 X, BcU32 Y, const RsColour& Colour )
{
	if( pTextureData_ != NULL && X < Header_.Width_ && Y < Header_.Height_ )
	{
		BcU32* pTextureData = (BcU32*)pTextureData_;
		BcU32 Index = X + Y * Header_.Width_;
		BcU32& Texel = pTextureData[ Index ];
		Texel = Colour.asABGR();
	}
}

//////////////////////////////////////////////////////////////////////////
// lock
void ScnTexture::lock()
{
	if( pTexture_ != NULL )
	{
		pTexture_->lockTexture();
	}
}

//////////////////////////////////////////////////////////////////////////
// unlock
void ScnTexture::unlock()
{
	if( pTexture_ != NULL )
	{
		pTexture_->unlockTexture();
	}
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
// setup
void ScnTexture::setup()
{
	if( CreateNewTexture_ == BcTrue )
	{
		// Destroy the old texture.
		if( pTexture_ != NULL )
		{
			RsCore::pImpl()->destroyResource( pTexture_ );
		}
		
		// Create new one immediately.
		pTexture_ = RsCore::pImpl()->createTexture( Header_.Width_,
												    Header_.Height_,
												    Header_.Levels_,
												    Header_.Format_,
												    pTextureData_ );
		CreateNewTexture_ = BcFalse;
	}
	else
	{
		RsCore::pImpl()->updateResource( pTexture_ );
	}
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
		// Request all texture levels.
		for( BcU32 iLevel = 0; iLevel < Header_.Levels_; ++iLevel )
		{
			requestChunk( ++ChunkIdx );
		}
		
		// We update the header, create a new texture rather than updating.
		CreateNewTexture_ = BcTrue;	
	}
	else if( ChunkID == BcHash( "body" ) )
	{
		// Grab pointer to data.
		BcAssert( pTextureData_ == NULL || pTextureData_ == pData );
		pTextureData_ = pData;
		
		// Setup.
		setup();
	}
}


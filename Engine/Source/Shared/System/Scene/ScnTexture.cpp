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

#include "ScnTexture.h"
#include "RsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#include "Img.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnTexture::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const std::string& FileName = Object[ "source" ].asString();

	// Add root dependancy.
	DependancyList.push_back( FileName );

	// Load texture from file and create the data for export.
	ImgImage* pImage = Img::load( FileName.c_str() );
	
	if( pImage != NULL )
	{
		// Encode the image as a format.
		BcU8* pEncodedImageData = NULL;
		BcU32 EncodedImageDataSize = 0;

		ImgEncodeFormat EncodeFormat;
		eRsTextureFormat TextureFormat;

		/*
		if( pImage->hasAlpha( 250 ) )
		{
			EncodeFormat = imgEF_DXT3;
			TextureFormat = rsTF_DXT3;
		}
		else
		*/
		{

			EncodeFormat = imgEF_DXT5;
			TextureFormat = rsTF_DXT5;
		}

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
			pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
			pFile_->addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );
					
			//
			return BcTrue;
		}
	}
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnTexture );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnTexture::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnTexture" )
		.field( "source",		csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise()
{
	// NULL internals.
	pTexture_ = NULL;
	pHeader_ = NULL;
	pTextureData_ = NULL;
	CreateNewTexture_ = BcTrue;
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
	return pHeader_->Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
BcU32 ScnTexture::getHeight() const
{
	return pHeader_->Height_;
}

//////////////////////////////////////////////////////////////////////////
// getTexel
RsColour ScnTexture::getTexel( BcU32 X, BcU32 Y ) const
{
	if( pTextureData_ != NULL && X < pHeader_->Width_ && Y < pHeader_->Height_ )
	{
		BcU32* pTextureData = (BcU32*)pTextureData_;
		BcU32 Index = X + Y * pHeader_->Width_;
		BcU32 Texel = pTextureData[ Index ] ;
		return RsColour( Texel );
	}
	
	return RsColour( 0.0f, 0.0f, 0.0f, 0.0f );
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
	// If we have a texture already, then we 
	// We've got a body chunk, so create internal resource.
	if( RsCore::pImpl() )
	{
		if( CreateNewTexture_ == BcTrue )
		{
			// Destroy the old texture.
			if( pTexture_ != NULL )
			{
				RsCore::pImpl()->destroyResource( pTexture_ );
			}
			
			// Create new one immediately.
			pTexture_ = RsCore::pImpl()->createTexture( pHeader_->Width_,
													    pHeader_->Height_,
													    pHeader_->Levels_,
													    pHeader_->Format_,
													    pTextureData_ );
			CreateNewTexture_ = BcFalse;
		}
		else
		{
			RsCore::pImpl()->updateResource( pTexture_ );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnTexture::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnTexture::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		// Grab pointer to header.
		pHeader_ = reinterpret_cast< THeader* >( pData );
				
		// Request all texture levels.
		for( BcU32 iLevel = 0; iLevel < pHeader_->Levels_; ++iLevel )
		{
			getChunk( ++ChunkIdx );
		}
		
		// We update the header, create a new texture rather than updating.
		CreateNewTexture_ = BcTrue;	
	}
	else if( pChunk->ID_ == BcHash( "body" ) )
	{
		// Grab pointer to data.
		BcAssert( pTextureData_ == NULL || pTextureData_ == pData );
		pTextureData_ = pData;
		
		// Setup.
		setup();
	}
}


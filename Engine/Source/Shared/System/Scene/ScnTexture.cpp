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
#endif

#ifdef PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnTexture::import( const Json::Value& Object )
{
	const std::string& FileName = Object[ "source" ].asString();

	// Load texture from file and create the data for export.
	ImgImage* pImage = ImgLoader::load( FileName.c_str() );
	
	if( pImage != NULL )
	{
		BcStream HeaderStream;
		BcStream BodyStream;
		
		// TODO: Use parameters to pick a format.
		THeader Header = { pImage->width(), pImage->height(), 1, rsTF_RGBA8 };
		HeaderStream << Header;
		
		// Write body.				
		for( BcU32 Y = 0; Y < pImage->height(); ++Y )
		{
			for( BcU32 X = 0; X < pImage->width(); ++X )
			{
				ImgColour Colour = pImage->getPixel( X, Y );
				
				BodyStream << Colour;
			}
		}
		
		// Add chunks and finish up.
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "body" ), BodyStream.pData(), BodyStream.dataSize() );
				
		//
		return BcTrue;
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
	return pTexture_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// getTexture
RsTexture* ScnTexture::getTexture()
{
	return pTexture_;
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
	pFile_->getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnTexture::fileChunkReady( const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		// Grab pointer to header.
		pHeader_ = reinterpret_cast< THeader* >( pData );
				
		// Request all texture levels.
		for( BcU32 iLevel = 0; iLevel < pHeader_->Levels_; ++iLevel )
		{
			pFile_->getChunk( iLevel + 1 );
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


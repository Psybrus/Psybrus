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

#include "System/Scene/Rendering/ScnTexture.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnTextureImport.h"
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
		
	auto& Class = ReRegisterClass< ScnTexture, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnTextureImport::StaticGetClass(), 0 ) );
#endif
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
			RsResourceBindFlags::SHADER_RESOURCE,
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


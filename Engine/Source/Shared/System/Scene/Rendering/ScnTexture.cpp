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
#include "System/Os/OsCore.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnTextureImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnTexture );

void ScnTexture::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pTexture_", &ScnTexture::pTexture_, bcRFF_TRANSIENT ),
		new ReField( "Header_", &ScnTexture::Header_, bcRFF_POD ),
		new ReField( "Width_", &ScnTexture::Width_ ),
		new ReField( "Height_", &ScnTexture::Height_ ),
		new ReField( "Depth_", &ScnTexture::Depth_ ),
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
// Ctor
ScnTexture::ScnTexture()
{
	pTexture_ = nullptr;
	pTextureData_ = nullptr;
	Width_ = 0;
	Height_ = 0;
	Depth_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTexture::ScnTexture( BcU32 Width, BcU32 Levels, RsTextureFormat Format )
{
	initialise( Width, Levels, Format );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTexture::ScnTexture( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format )
{
	initialise( Width, Height, Levels, Format );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTexture::ScnTexture( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, RsTextureFormat Format )
{
	initialise( Width, Height, Depth, Levels, Format );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
ScnTexture::~ScnTexture()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnTexture::initialise( BcU32 Width, BcU32 Levels, RsTextureFormat Format )
{
	pTexture_ = nullptr;
	pTextureData_ = nullptr;

	Width_ = Header_.Width_ = Width;
	Height_ = Header_.Height_ = 0;
	Depth_ = Header_.Depth_ = 0;
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
	pTexture_ = nullptr;
	pTextureData_ = nullptr;

	Width_ = Header_.Width_ = Width;
	Height_ = Header_.Height_ = Height;
	Depth_ = Header_.Depth_ = 0;
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
	pTexture_ = nullptr;
	pTextureData_ = nullptr;

	Width_ = Header_.Width_ = Width;
	Height_ = Header_.Height_ = Height;
	Depth_ = Header_.Depth_ = Depth;
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
	// If width or height is a fraction of client size, then register
	// for recreation.
	if( Header_.Width_ < 0 || Header_.Height_ < 0 )
	{
		OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
			[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
			{
				recreate();
				return evtRET_PASS;
			} );
	}

	recreate();
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnTexture::destroy()
{
	OsCore::pImpl()->unsubscribeAll( this );

	RsCore::pImpl()->destroyResource( pTexture_ );
	pTexture_ = nullptr;

	// Free if it's user created.
	if( Header_.Editable_ )
	{
		BcMemFree( pTextureData_ );
		pTextureData_ = nullptr;
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
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
BcU32 ScnTexture::getHeight() const
{
	return Height_;
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
// recreate
void ScnTexture::recreate()
{
	RsContext* Context = RsCore::pImpl()->getContext( nullptr );

	Width_ = Header_.Width_;
	Height_ = Header_.Height_;
	Depth_ = Header_.Depth_;
	
	// If a target, use negative width + height as multiples of resolution.
	if( Header_.RenderTarget_ || Header_.DepthStencilTarget_ )
	{
		if( Header_.Width_ < 0 )
		{
			Width_ = Context->getWidth() / -Header_.Width_;
		}

		if( Header_.Height_ < 0 )
		{
			Height_ = Context->getHeight() / -Header_.Height_;
		}
	}

	BcAssert( Width_ >= 0 );
	BcAssert( Height_ >= 0 );
	BcAssert( Depth_ >= 0 );

	// Set to valid minimums.
	if( Width_ == 0 )
	{
		Width_ = 1;
	}
	if( Height_ == 0 )
	{
		Height_ = 1;
	}
	if( Depth_ == 0 )
	{
		Depth_ = 1;
	}

	// Allocate if editable.
	if( Header_.Editable_ )
	{
		BcMemFree( pTextureData_ );

		// Allocate to a 4k alignment.
		pTextureData_ = BcMemAlign(
			RsTextureFormatSize( 
				Header_.Format_, 
				Width_, 
				Height_,
				Depth_, 
				Header_.Levels_ ), 4096 );
	}

	// Create new one immediately.
	auto CreationFlags = Header_.Editable_ ? RsResourceCreationFlags::DYNAMIC : RsResourceCreationFlags::STATIC;
	auto BindFlags = RsResourceBindFlags::SHADER_RESOURCE |
				( Header_.RenderTarget_ ? RsResourceBindFlags::RENDER_TARGET : RsResourceBindFlags::NONE ) |
				( Header_.DepthStencilTarget_ ? RsResourceBindFlags::DEPTH_STENCIL : RsResourceBindFlags::NONE );

	// Free old.
	if( pTexture_ != nullptr )
	{
		RsCore::pImpl()->destroyResource( pTexture_ );
		pTexture_ = nullptr;
	}

	// Create new.
	pTexture_ = RsCore::pImpl()->createTexture( 
		RsTextureDesc( 
			Header_.Type_, 
			CreationFlags,
			BindFlags,
			Header_.Format_,
			Header_.Levels_,
			Width_,
			Height_,
			Depth_ ) );

	// Upload texture data.
	if( !Header_.RenderTarget_ && !Header_.DepthStencilTarget_ )
	{
		BcU8* TextureData = reinterpret_cast< BcU8* >( pTextureData_ );
		BcU32 Width = Width_;
		BcU32 Height = Height_;
		BcU32 Depth = Depth_;
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
		/*
		// Request all texture levels.
		for( BcU32 iLevel = 0; iLevel < Header_.Levels_; ++iLevel )
		{
			requestChunk( ++ChunkIdx );
		}
		*/

		if( Header_.Editable_ == BcFalse &&
			Header_.RenderTarget_ == BcFalse &&
			Header_.DepthStencilTarget_ == BcFalse )
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


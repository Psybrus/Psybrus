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
#include "System/SysKernel.h"

#include "Base/BcMath.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnTextureImport.h"
#endif

#include "System/Debug/DsImGuiFieldEditor.h"

#define DO_TEXTURE_COPY_TEST ( 0 )

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnTexture );

void ScnTexture::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
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

	// Add editor.
	Class.addAttribute( 
		new DsImGuiFieldEditor( 
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				ScnTexture* Value = (ScnTexture*)Object;
				if( Value != nullptr )
				{
					ImGui::Text( "Width: %u", Value->Width_ );
					ImGui::Text( "Height: %u", Value->Height_ );
					ImGui::Text( "Depth: %u", Value->Depth_ );
					ImGui::Text( "Format: TODO" );
					MaVec2d Size( Value->Width_, Value->Height_ );
					const auto WidthRequirement = 256.0f;
					Size *= WidthRequirement / Size.x();
					ImGui::Image( Value->getTexture(), Size );
				}
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnTexture::ScnTexture()
{
	memset( &Header_, 0, sizeof( Header_ ) );
	pTextureData_ = nullptr;
	Width_ = 0;
	Height_ = 0;
	Depth_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Dtpr
//virtual
ScnTexture::~ScnTexture()
{

}

//////////////////////////////////////////////////////////////////////////
// New
//static
ScnTexture* ScnTexture::New( const RsTextureDesc& Desc, const char* DebugName )
{
	auto Texture = new ScnTexture();
	Texture->Texture_ = RsCore::pImpl()->createTexture( Desc, DebugName );
	Texture->pTextureData_ = nullptr;
	Texture->Width_ = Texture->Texture_->getDesc().Width_;
	Texture->Height_ = Texture->Texture_->getDesc().Height_;
	Texture->Depth_ = Texture->Texture_->getDesc().Depth_;
	Texture->Header_.Width_ = Texture->Width_;
	Texture->Header_.Height_ = Texture->Height_;
	Texture->Header_.Depth_ = Texture->Depth_;
	Texture->Header_.Levels_ = Texture->Texture_->getDesc().Levels_;
	Texture->Header_.Type_ = Texture->Texture_->getDesc().Type_;
	Texture->Header_.Format_ = Texture->Texture_->getDesc().Format_;
	Texture->Header_.Editable_ = BcFalse;
	Texture->Header_.RenderTarget_ = ( Texture->Texture_->getDesc().BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) == RsResourceBindFlags::NONE;
	Texture->Header_.DepthStencilTarget_ = ( Texture->Texture_->getDesc().BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) == RsResourceBindFlags::NONE;
	Texture->markCreate();
	return Texture;
}

//////////////////////////////////////////////////////////////////////////
// New1D
//static
ScnTexture* ScnTexture::New1D( BcU32 Width, BcU32 Levels, RsTextureFormat Format, const char* DebugName )
{
	auto Texture = new ScnTexture();
	Texture->pTextureData_ = nullptr;
	Texture->Width_ = Texture->Header_.Width_ = Width;
	Texture->Height_ = Texture->Header_.Height_ = 0;
	Texture->Depth_ = Texture->Header_.Depth_ = 0;
	Texture->Header_.Width_ = Texture->Width_;
	Texture->Header_.Height_ = Texture->Height_;
	Texture->Header_.Depth_ = Texture->Depth_;
	Texture->Header_.Levels_ = Levels;
	Texture->Header_.Type_ = RsTextureType::TEX1D;
	Texture->Header_.Format_ = Format;
	Texture->Header_.Editable_ = BcTrue;
	Texture->Header_.RenderTarget_ = BcFalse;
	Texture->Header_.DepthStencilTarget_ = BcFalse;
	Texture->markCreate();
	return Texture;
}

//////////////////////////////////////////////////////////////////////////
// New2D
//static
ScnTexture* ScnTexture::New2D( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format, const char* DebugName )
{
	auto Texture = new ScnTexture();
	Texture->pTextureData_ = nullptr;
	Texture->Width_ = Texture->Header_.Width_ = Width;
	Texture->Height_ = Texture->Header_.Height_ = Height;
	Texture->Depth_ = Texture->Header_.Depth_ = 0;
	Texture->Header_.Width_ = Texture->Width_;
	Texture->Header_.Height_ = Texture->Height_;
	Texture->Header_.Depth_ = Texture->Depth_;
	Texture->Header_.Levels_ = Levels;
	Texture->Header_.Type_ = RsTextureType::TEX2D;
	Texture->Header_.Format_ = Format;
	Texture->Header_.Editable_ = BcTrue;
	Texture->Header_.RenderTarget_ = BcFalse;
	Texture->Header_.DepthStencilTarget_ = BcFalse;
	Texture->markCreate();
	return Texture;
}

//////////////////////////////////////////////////////////////////////////
// New3D
//static
ScnTexture* ScnTexture::New3D( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, RsTextureFormat Format, const char* DebugName )
{
	auto Texture = new ScnTexture();
	Texture->pTextureData_ = nullptr;
	Texture->Width_ = Texture->Header_.Width_ = Width;
	Texture->Height_ = Texture->Header_.Height_ = Height;
	Texture->Depth_ = Texture->Header_.Depth_ = Depth;
	Texture->Header_.Width_ = Texture->Width_;
	Texture->Header_.Height_ = Texture->Height_;
	Texture->Header_.Depth_ = Texture->Depth_;
	Texture->Header_.Levels_ = Levels;
	Texture->Header_.Type_ = RsTextureType::TEX3D;
	Texture->Header_.Format_ = Format;
	Texture->Header_.Editable_ = BcTrue;
	Texture->Header_.RenderTarget_ = BcFalse;
	Texture->Header_.DepthStencilTarget_ = BcFalse;
	Texture->markCreate();
	return Texture;
}

//////////////////////////////////////////////////////////////////////////
// NewCube
//static
ScnTexture* ScnTexture::NewCube( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format, const char* DebugName )
{
	auto Texture = new ScnTexture();
	Texture->pTextureData_ = nullptr;
	Texture->Width_ = Texture->Header_.Width_ = Width;
	Texture->Height_ = Texture->Header_.Height_ = Height;
	Texture->Depth_ = Texture->Header_.Depth_ = 0;
	Texture->Header_.Width_ = Texture->Width_;
	Texture->Header_.Height_ = Texture->Height_;
	Texture->Header_.Depth_ = Texture->Depth_;
	Texture->Header_.Levels_ = Levels;
	Texture->Header_.Type_ = RsTextureType::TEXCUBE;
	Texture->Header_.Format_ = Format;
	Texture->Header_.Editable_ = BcTrue;
	Texture->Header_.RenderTarget_ = BcFalse;
	Texture->Header_.DepthStencilTarget_ = BcFalse;
	Texture->markCreate();
	return Texture;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnTexture::create()
{
	// If width or height is a fraction of client size, then register
	// for recreation.
	if( Header_.Width_ <= 0 || Header_.Height_ <= 0 )
	{
		OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
			[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
			{
				recreate();
				return evtRET_PASS;
			} );
	}

	if( Texture_ == nullptr )
	{
		recreate();
	}
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnTexture::destroy()
{
	OsCore::pImpl()->unsubscribeAll( this );

	Texture_.reset();
}

//////////////////////////////////////////////////////////////////////////
// getTexture
RsTexture* ScnTexture::getTexture()
{
	return Texture_.get();
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
// isClientDependent
bool ScnTexture::isClientDependent() const
{
	return Header_.Width_ <= 0 || Header_.Height_ <= 0;
}

//////////////////////////////////////////////////////////////////////////
// getRect
//virtual
const ScnRect& ScnTexture::getRect( BcU32 Idx ) const
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
BcU32 ScnTexture::noofRects() const
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
// recreate
void ScnTexture::recreate()
{
	auto* Context = RsCore::pImpl()->getContext( nullptr );
	const auto& Features = Context->getFeatures();

	Width_ = Header_.Width_;
	Height_ = Header_.Height_;
	Depth_ = Header_.Depth_;
	
	// If a target, use negative width + height as multiples of resolution.
	if( Header_.RenderTarget_ || Header_.DepthStencilTarget_ )
	{
		auto* Client = Context->getClient();
		if( Header_.Width_ <= 0 )
		{
			Width_ = Client->getWidth() >> -Header_.Width_;
		}

		if( Header_.Height_ <= 0 )
		{
			Height_ = Client->getHeight() >> -Header_.Height_;
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

	// If texture is NPOT and we don't support it, round up size and warn.
	// We don't want to crash out if possible.
	const BcBool IsNPOT = !BcPot( Width_ ) || !BcPot( Height_ ) || !BcPot( Depth_ );
	if( IsNPOT && Features.NPOTTextures_ == false )
	{
		PSY_LOG( "WARNING: Rounding down texture \"%s\" to a power of two.", (*getName()).c_str() );
		Width_ = BcMax( BcPotNext( Width_ ), 1 );
		Height_ = BcMax( BcPotNext( Height_ ), 1 );
		Depth_ = BcMax( BcPotNext( Depth_ ), 1 );
	}

	// Create new one immediately.
	auto CreationFlags = Header_.Editable_ ? RsResourceCreationFlags::DYNAMIC : RsResourceCreationFlags::STATIC;
	RsResourceBindFlags BindFlags = RsResourceBindFlags::SHADER_RESOURCE;

	if( Header_.RenderTarget_ )
	{
		BindFlags = RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET;
	}
	if( Header_.DepthStencilTarget_ )
	{
		BindFlags = RsResourceBindFlags::DEPTH_STENCIL;

		if( Features.DepthTextures_ )
		{
			BindFlags = BindFlags | RsResourceBindFlags::SHADER_RESOURCE;
		}
	}

	// Free old.
	Texture_.reset();

	BcU32 SkipMips = 0;

	// Create new.
	Texture_ = RsCore::pImpl()->createTexture( 
		RsTextureDesc( 
			Header_.Type_, 
			CreationFlags,
			BindFlags,
			Header_.Format_,
			Header_.Levels_ - SkipMips,
			Width_ >> SkipMips,
			Height_ >> SkipMips,
			Depth_ ),
		getFullName().c_str() );

#if DO_TEXTURE_COPY_TEST
	auto StagingTexture = RsCore::pImpl()->createTexture( 
		RsTextureDesc( 
			Header_.Type_, 
			RsResourceCreationFlags::STREAM,
			RsResourceBindFlags::NONE,
			Header_.Format_,
			Header_.Levels_ - SkipMips,
			Width_ >> SkipMips,
			Height_ >> SkipMips,
			Depth_ ),
		getFullName().c_str() );
#else
	auto& StagingTexture = Texture_;
#endif

	// Upload texture data.
	if( pTextureData_ != nullptr )
	{
		BcU8* TextureData = reinterpret_cast< BcU8* >( pTextureData_ );
		BcU32 Width = Width_;
		BcU32 Height = Height_;
		BcU32 Depth = Depth_;
		for( BcU32 LevelIdx = 0; LevelIdx < Header_.Levels_; ++LevelIdx )
		{
			const auto SourcePitch = 
				RsTexturePitch( Header_.Format_, Width, Height );

			const auto BlockInfo =
				RsTextureBlockInfo( Header_.Format_ );

			const auto SliceSize = 
				RsTextureFormatSize( 
					Header_.Format_, 
					Width, 
					Height,
					Depth, 
					1 );

			if( LevelIdx >= SkipMips )
			{
				auto Slice = StagingTexture->getSlice( LevelIdx - SkipMips );

				RsCore::pImpl()->updateTexture( 
					StagingTexture.get(),
					Slice,
					RsResourceUpdateFlags::ASYNC,
					[ this, TextureData, SourcePitch, SliceSize, Height, BlockInfo ]( RsTexture* Texture, const RsTextureLock& Lock )
					{
						BcAssert( Lock.Buffer_ );
						BcAssert( Lock.Pitch_ >= SourcePitch );
						BcAssert( Lock.SlicePitch_ >= SliceSize );
						const auto Rows = Height / BlockInfo.Height_;
						for( BcU32 Row = 0; Row < Rows; ++Row )
						{
							BcU8* DestData = reinterpret_cast< BcU8* >( Lock.Buffer_ ) + ( Lock.Pitch_ * Row );
							memcpy( DestData, TextureData + ( SourcePitch * Row ), SourcePitch );
						}
					} );
			}

			// Down a level.
			Width = BcMax( 1, Width >> 1 );
			Height = BcMax( 1, Height >> 1 );
			Depth = BcMax( 1, Depth >> 1 );

			// Advance texture data.
			TextureData += SliceSize;
		}

#if DO_TEXTURE_COPY_TEST
		// Perform copy texture.
		SysKernel::pImpl()->pushFunctionJob( 
			RsCore::JOB_QUEUE_ID,
			[ 
				SourceTexture = StagingTexture.get(),
				DestTexture = Texture_.get()
			]
			()
			{
				auto Context = SourceTexture->getContext();
				Context->copyTexture(
					SourceTexture,
					DestTexture );
			} );
#endif
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


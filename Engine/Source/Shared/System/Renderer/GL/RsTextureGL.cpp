#include "System/Renderer/GL/RsTextureGL.h"
#include "System/Renderer/GL/RsContextGL.h"
#include "System/Renderer/GL/RsUtilsGL.h"
#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureGL::RsTextureGL( RsTexture* Parent, RsTextureGL::ResourceType ResourceType ):
	Parent_( Parent ),
	ResourceType_( ResourceType )
{
	Parent_->setHandle( this );
	if( ResourceType_ == RsTextureGL::ResourceType::TEXTURE )
	{
		const auto& TextureDesc = Parent_->getDesc();
		auto ContextGL = static_cast< RsContextGL* >( Parent_->getContext() );
	
		// Get buffer type for GL.
		auto TypeGL = RsUtilsGL::GetTextureType( TextureDesc.Type_ );

		// Get usage flags for GL.
		GLuint UsageFlagsGL = 0;
	
		// Data update frequencies.
		if( ( TextureDesc.CreationFlags_ & RsResourceCreationFlags::STATIC ) != RsResourceCreationFlags::NONE )
		{
			UsageFlagsGL |= GL_STATIC_DRAW;
		}
		else if( ( TextureDesc.CreationFlags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
		{
			UsageFlagsGL |= GL_DYNAMIC_DRAW;
		}
		else if( ( TextureDesc.CreationFlags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
		{
			UsageFlagsGL |= GL_STREAM_DRAW;
		}

		GL( GenTextures( 1, &Handle_ ) );

		// Bind texture.
		ContextGL->bindTexture( 0, Parent_ );
		
#if !defined( RENDER_USE_GLES )
		// Set max levels.
		GL( TexParameteri( TypeGL, GL_TEXTURE_MAX_LEVEL, TextureDesc.Levels_ - 1 ) );

		// Set compare mode to none.
		if( TextureDesc.Format_ == RsResourceFormat::D16_UNORM ||
			TextureDesc.Format_ == RsResourceFormat::D24_UNORM_S8_UINT ||
			TextureDesc.Format_ == RsResourceFormat::D32_FLOAT )
		{
			GL( TexParameteri( TypeGL, GL_TEXTURE_COMPARE_MODE, GL_NONE ) );
			
			GL( TexParameteri( TypeGL, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ) );
			
		}
#endif

		// Instantiate levels.
		BcU32 Width = TextureDesc.Width_;
		BcU32 Height = TextureDesc.Height_;
		BcU32 Depth = TextureDesc.Depth_;
		if( TextureDesc.Type_ != RsTextureType::TEXCUBE )
		{
			for( BcU32 LevelIdx = 0; LevelIdx < TextureDesc.Levels_; ++LevelIdx )
			{
				auto TextureSlice = Parent_->getSlice( LevelIdx );

				// Load slice.
				loadTexture( TextureSlice, 
					RsResourceFormatSize( TextureDesc.Format_, Width, Height, Depth, 1 ), 
					nullptr );
				// TODO: Error checking on loadTexture.

				// Down a power of two.
				Width = BcMax( 1, Width >> 1 );
				Height = BcMax( 1, Height >> 1 );
				Depth = BcMax( 1, Depth >> 1 );
			}
		}
		else
		{
			for( BcU32 LevelIdx = 0; LevelIdx < TextureDesc.Levels_; ++LevelIdx )
			{
				for( BcU32 FaceIdx = 0; FaceIdx < 6; ++FaceIdx )
				{
					auto TextureSlice = Parent_->getSlice( LevelIdx, RsTextureFace( FaceIdx ) );

					// Load slice.
					loadTexture( TextureSlice, 0, nullptr );
					// TODO: Error checking on loadTexture.

					// Down a power of two.
					Width = BcMax( 1, Width >> 1 );
					Height = BcMax( 1, Height >> 1 );
				}
			}
		}

#if !defined( RENDER_USE_GLES ) && !PSY_PRODUCTION
		if( GLEW_KHR_debug )
		{
			glObjectLabel( GL_TEXTURE, Handle_, BcStrLength( Parent->getDebugName() ), Parent->getDebugName() );
		}
#endif
	}

}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsTextureGL::~RsTextureGL()
{
	GL( DeleteTextures( 1, &Handle_ ) );
}

////////////////////////////////////////////////////////////////////////////////
// loadTexture
void RsTextureGL::loadTexture(
		const RsTextureSlice& Slice,
		BcU32 DataSize,
		void* Data )
{
	BcAssert( ResourceType_ == ResourceType::TEXTURE );
	const auto& TextureDesc = Parent_->getDesc();
	auto ContextGL = static_cast< RsContextGL* >( Parent_->getContext() );

	// Get buffer type for GL.
	auto TypeGL = RsUtilsGL::GetTextureType( TextureDesc.Type_ );

	ContextGL->bindTexture( 0, Parent_ );
	
	BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
	BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
	BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );

	const auto& FormatGL = RsUtilsGL::GetResourceFormat( TextureDesc.Format_ );
	
	if( FormatGL.Compressed_ == BcFalse )
	{
		switch( TextureDesc.Type_ )
		{
		case RsTextureType::TEX1D:
#if !defined( RENDER_USE_GLES )
			GL( TexImage1D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data ) );
#else
			// TODO ES2.
			BcBreakpoint;
#endif
			break;

		case RsTextureType::TEX2D:
			GL( TexImage2D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data ) );
			break;

		case RsTextureType::TEX3D:
#if !defined( RENDER_USE_GLES )
			GL( TexImage3D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				Depth,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data ) );
#else
			// TODO ES2.
			BcBreakpoint;
#endif
			break;

		case RsTextureType::TEXCUBE:
			GL( TexImage2D( 
				RsUtilsGL::GetTextureFace( Slice.Face_ ),
				Slice.Level_,
				FormatGL.InternalFormat_,
				Width,
				Height,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data ) );
			break;

		default:
			BcBreakpoint;
		}
	}
	else
	{
		if( Data != nullptr )
		{
			switch( TextureDesc.Type_ )
			{
#if !defined( RENDER_USE_GLES )
			case RsTextureType::TEX1D:
				GL( CompressedTexImage1D( 
					TypeGL, 
					Slice.Level_,
					FormatGL.InternalFormat_,
					Width,
					0,
					DataSize,
					Data ) );
				break;
#endif // !defined( RENDER_USE_GLES )

			case RsTextureType::TEX2D:
				GL( CompressedTexImage2D( 
					TypeGL, 
					Slice.Level_, 
					FormatGL.InternalFormat_,
					Width,
					Height,
					0,
					DataSize,
					Data ) );
				break;

#if !defined( RENDER_USE_GLES )
			case RsTextureType::TEX3D:
				GL( CompressedTexImage3D( 
					TypeGL, 
					Slice.Level_, 
					FormatGL.InternalFormat_,
					Width,
					Height,
					Depth,
					0,
					DataSize,
					Data ) );
				break;

			case RsTextureType::TEXCUBE:
				PSY_LOG("TODO: Investigate why this doesn't work.");
				GL( CompressedTexImage2D( 
					RsUtilsGL::GetTextureFace( Slice.Face_ ),
					Slice.Level_,
					FormatGL.InternalFormat_,
					Width,
					Height,
					0,
					DataSize,
					Data ) );
				break;
#endif

			default:
				BcBreakpoint;
			}
		}
	}
}


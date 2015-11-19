#include "System/Renderer/GL/RsTextureGL.h"
#include "System/Renderer/GL/RsContextGL.h"
#include "System/Renderer/GL/RsUtilsGL.h"
#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureGL::RsTextureGL( RsTexture* Parent ):
	Parent_( Parent )
{
	Parent_->setHandle( this );
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
	if( TextureDesc.Format_ == RsTextureFormat::D16 ||
		TextureDesc.Format_ == RsTextureFormat::D24 ||
		TextureDesc.Format_ == RsTextureFormat::D32 ||
		TextureDesc.Format_ == RsTextureFormat::D24S8 )
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
			loadTexture( TextureSlice, 0, nullptr );
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
				auto TextureSlice = Parent_->getSlice( LevelIdx, RsTextureFace( FaceIdx + 1 ) );

				// Load slice.
				loadTexture( TextureSlice, 0, nullptr );
				// TODO: Error checking on loadTexture.

				// Down a power of two.
				Width = BcMax( 1, Width >> 1 );
				Height = BcMax( 1, Height >> 1 );
			}
		}
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
	const auto& TextureDesc = Parent_->getDesc();
	auto ContextGL = static_cast< RsContextGL* >( Parent_->getContext() );

	// Get buffer type for GL.
	auto TypeGL = RsUtilsGL::GetTextureType( TextureDesc.Type_ );

	ContextGL->bindTexture( 0, Parent_ );
	
	BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
	BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
	BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );

	const auto& FormatGL = RsUtilsGL::GetTextureFormat( TextureDesc.Format_ );
	
#if defined( RENDER_USE_GLES )
	// TODO: Fix this properly.
	if( !FormatGL.Compressed_ )
	{
		if( Slice.Level_ > 0 )
		{
			if( Slice.Level_ > 1 )
			{
				return;
			}
			glGenerateMipmap( TypeGL );
			if( glGetError() != 0 )
			{
				PSY_LOG( "ERROR: Attempting to create mipmaps for texture (%u levels), but glGenerateMipMap failed.", TextureDesc.Levels_ );
			}
			return;
		}
	}
#endif

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
		// TODO: More intrusive checking of format.
		if( DataSize == 0 || Data == nullptr )
		{
			return;
		}

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
			GL( CompressedTexImage2D( 
				RsUtilsGL::GetTextureFace( Slice.Face_ ),
				Slice.Level_,
				FormatGL.InternalFormat_,
				Width,
				Height,
				0,
				DataSize,
				Data ) );
#endif

		default:
			BcBreakpoint;
		}
	}
}


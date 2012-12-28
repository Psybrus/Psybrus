/**************************************************************************
*
* File:		RsTextureGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsTextureGL.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/GL/RsCoreImplGL.h"

// TODO: Move into a shared location.
static GLenum gTextureTypes[] = 
{
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP
};

//////////////////////////////////////////////////////////////////////////
// RsTextureGL
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureGL::RsTextureGL( BcU32 Width, BcU32 Levels, eRsTextureFormat Format, void* pTextureData )
{
	// Setup parameters.
	Width_ = Width;
	Height_ = 1;
	Depth_ = 1;
	Levels_ = Levels;
	Type_ = rsTT_1D;
	Format_ = Format;
	Locked_ = BcFalse;
	pData_ = pTextureData;
	DataSize_ = RsTextureFormatSize( Format_, Width_, Height_, Depth_, Levels_ );
	
	// Create data if we need to.
	if( pData_ == NULL )
	{		
		pData_ = new BcU8[ DataSize_ ];
		DeleteData_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureGL::RsTextureGL( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pTextureData )
{
	// Setup parameters.
	Width_ = Width;
	Height_ = Height;
	Depth_ = 1;
	Levels_ = Levels;
	Type_ = rsTT_2D;
	Format_ = Format;
	Locked_ = BcFalse;
	pData_ = pTextureData;
	DataSize_ = RsTextureFormatSize( Format_, Width_, Height_, Depth_, Levels_ );
	
	// Create data if we need to.
	if( pData_ == NULL )
	{		
		pData_ = new BcU8[ DataSize_ ];
		DeleteData_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureGL::RsTextureGL( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, eRsTextureFormat Format, void* pTextureData )
{
	// Setup parameters.
	Width_ = Width;
	Height_ = Height;
	Depth_ = Depth;
	Levels_ = Levels;
	Type_ = rsTT_3D;
	Format_ = Format;
	Locked_ = BcFalse;
	pData_ = pTextureData;
	DataSize_ = RsTextureFormatSize( Format_, Width_, Height_, Depth_, Levels_ );
	
	// Create data if we need to.
	if( pData_ == NULL )
	{		
		pData_ = new BcU8[ DataSize_ ];
		DeleteData_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsTextureGL::~RsTextureGL()
{

}

//////////////////////////////////////////////////////////////////////////
// width
//virtual
BcU32 RsTextureGL::width() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// height
//virtual
BcU32 RsTextureGL::height() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// depth
//virtual
BcU32 RsTextureGL::depth() const
{
	return Depth_;
}

//////////////////////////////////////////////////////////////////////////
// levels
//virtual
BcU32 RsTextureGL::levels() const
{
	return Levels_;
}

//////////////////////////////////////////////////////////////////////////
// type
//virtual
eRsTextureType RsTextureGL::type() const
{
	return Type_;	
}

//////////////////////////////////////////////////////////////////////////
// format
//virtual
eRsTextureFormat RsTextureGL::format() const
{
	return Format_;
}


//////////////////////////////////////////////////////////////////////////
// lockTexture
//virtual
void* RsTextureGL::lockTexture()
{
	wait();
	return pData_;
}

//////////////////////////////////////////////////////////////////////////
// unlockTexture
//virtual
void RsTextureGL::unlockTexture()
{
	UpdateSyncFence_.increment();
	RsCore::pImpl()->updateResource( this );
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsTextureGL::create()
{
	// Create GL texture.
	GLuint Handle;
	glGenTextures( 1, &Handle );
	setHandle( Handle );
	
	if( Handle != 0 )
	{
		GLenum Error = glGetError();

		UpdateSyncFence_.increment();

		// Update.
		update();

		// Destroy if there is a failure.
		Error = glGetError();
		if ( Error != GL_NO_ERROR )
		{
			destroy();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// update
void RsTextureGL::update()
{
	// Bind and upload.
	GLuint Handle = getHandle< GLuint >();
	
	GLenum TextureType = gTextureTypes[ Type_ ];

	glBindTexture( TextureType, Handle );
	RsGLCatchError;

	switch( Type_ )
	{
	case rsTT_1D:
		loadTexture1D();
		break;
	
	case rsTT_2D:
		loadTexture2D();
		break;

	case rsTT_3D:
		loadTexture3D();
		break;

	case rsTT_CUBEMAP:
		loadTextureCubeMap();
		break;
	
	default:
		BcBreakpoint;
		break;
	}

	GLenum Error = glGetError();
	BcAssertMsg( Error == 0, "RsTextureGL: Error (0x%x) creating texture (%ux%ux%u, format %u, %u bytes).\n", Error, Width_, Height_, Depth_, Format_, DataSize_ );

	UpdateSyncFence_.decrement();
	
	// Invalidate texture state.
	RsStateBlock* pStateBlock = RsCore::pImpl()->getStateBlock();
	pStateBlock->invalidateTextureState();
}

////////////////////////////////////////////////////////////////////////////////
// destroy
void RsTextureGL::destroy()
{
	// Check that we haven't already freed it.
	GLuint Handle = getHandle< GLuint >();
	if( Handle != 0 )
	{
		// Delete it.
		glDeleteTextures( 1, &Handle );
		setHandle< GLuint >( 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// loadTexture1D
void RsTextureGL::loadTexture1D()
{
	// Call the appropriate method to load the texture.
	switch( Format_ )
	{
	case rsTF_RGB8:
		glTexImage1D( GL_TEXTURE_1D, 0, GL_RGB, Width_, 0, GL_RGB, GL_UNSIGNED_BYTE, pData_ );	
		break;

	case rsTF_RGBA8:
		glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, Width_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData_ );	
		break;

	case rsTF_I8:
		glTexImage1D( GL_TEXTURE_1D, 0, GL_INTENSITY8, Width_, 0, GL_INTENSITY8, GL_UNSIGNED_BYTE, pData_ );
		break;

	default:
		BcBreakpoint; // Unsupported format for platform.
	}
}

////////////////////////////////////////////////////////////////////////////////
// loadTexture2D
void RsTextureGL::loadTexture2D()
{
	BcU8* pData = reinterpret_cast< BcU8* >( pData_ );
	BcU32 Width = Width_;
	BcU32 Height = Height_;

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, Levels_ - 1 );

	for( BcU32 Idx = 0; Idx < Levels_; ++Idx )
	{
		BcU32 LevelBytes = RsTextureFormatSize( Format_, Width, Height, 1, 1 );

		// Call the appropriate method to load the texture.
		switch( Format_ )
		{
		case rsTF_RGB8:
			glTexImage2D( GL_TEXTURE_2D, Idx, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pData );	
			break;

		case rsTF_RGBA8:
			glTexImage2D( GL_TEXTURE_2D, Idx, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData );	
			break;

		case rsTF_I8:
			glTexImage2D( GL_TEXTURE_2D, Idx, GL_INTENSITY8, Width, Height, 0, GL_INTENSITY8, GL_UNSIGNED_BYTE, pData );
			break;

		case rsTF_DXT1:
			glCompressedTexImage2D( GL_TEXTURE_2D, Idx, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, Width, Height, 0, LevelBytes, pData );
			break;

		case rsTF_DXT3:
			glCompressedTexImage2D( GL_TEXTURE_2D, Idx, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, Width, Height, 0, LevelBytes, pData );
			break;

		case rsTF_DXT5:
			glCompressedTexImage2D( GL_TEXTURE_2D, Idx, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, Width, Height, 0, LevelBytes, pData );
			break;

		default:
			BcBreakpoint; // Unsupported format for platform.
		}

		GLenum Error = glGetError();
		BcAssertMsg( Error == 0, "RsTextureGL: Error (0x%x) creating texture (%ux%ux%u, format %u, %u bytes).\n", Error, Width, Height, 1, Format_, LevelBytes );
	
		Height >>= 1;
		Width >>= 1;
		pData += LevelBytes;
	}
}

////////////////////////////////////////////////////////////////////////////////
// loadTexture3D
void RsTextureGL::loadTexture3D()
{
		// Call the appropriate method to load the texture.
	switch( Format_ )
	{
	case rsTF_RGB8:
		glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB, Width_, Height_, Depth_, 0, GL_RGB, GL_UNSIGNED_BYTE, pData_ );	
		break;

	case rsTF_RGBA8:
		glTexImage3D( GL_TEXTURE_3D, 0, GL_RGBA, Width_, Height_, Depth_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData_ );	
		break;

	case rsTF_I8:
		glTexImage3D( GL_TEXTURE_3D, 0, GL_INTENSITY8, Width_, Height_, Depth_, 0, GL_INTENSITY8, GL_UNSIGNED_BYTE, pData_ );
		break;

	default:
		BcBreakpoint; // Unsupported format for platform.
	}
}

////////////////////////////////////////////////////////////////////////////////
// loadTextureCubeMap
void RsTextureGL::loadTextureCubeMap()
{
	BcBreakpoint;
}

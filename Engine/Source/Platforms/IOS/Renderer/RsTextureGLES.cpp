/**************************************************************************
*
* File:		RsTextureGLES.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsTextureGLES.h"

#include "RsCore.h"
#include "RsCoreImplGLES.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureGLES
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureGLES::RsTextureGLES( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pTextureData )
{
	// Setup parameters.
	Width_ = Width;
	Height_ = Height;
	Levels_ = Levels;
	Format_ = Format;
	Locked_ = BcFalse;
	pData_ = pTextureData;
	DataSize_ = RsTextureFormatSize( Format, Width, Height, Levels );
	
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
RsTextureGLES::~RsTextureGLES()
{

}

//////////////////////////////////////////////////////////////////////////
// width
//virtual
BcU32 RsTextureGLES::width() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// height
//virtual
BcU32 RsTextureGLES::height() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// lockTexture
//virtual
void* RsTextureGLES::lockTexture()
{
	BcAssert( bLocked_ == BcFalse );
	
	if( Locked_ == BcFalse )
	{
		return pData_;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// unlockTexture
//virtual
void RsTextureGLES::unlockTexture()
{
	BcAssert( Locked_ == BcTrue );

	// Upload new texture data.
	if( Locked_ == BcTrue )
	{
		Locked_ = BcFalse;
		//Dirty_ = BcTrue;
	}
}

////////////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsTextureGLES::create()
{
	// Create GLES texture.
	glGenTextures( 1, &Handle_ );
	
	if( Handle_ != 0 )
	{
		GLenum Error = glGetError();
		
		// Bind and upload.
		glBindTexture( GL_TEXTURE_2D, Handle_ );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, Width_, Height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData_ );		
		glBindTexture( GL_TEXTURE_2D, 0 );

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
void RsTextureGLES::update()
{

}

////////////////////////////////////////////////////////////////////////////////
// destroy
void RsTextureGLES::destroy()
{
	// Check that we haven't already freed it.
	if( Handle_ != 0 )
	{
		// Delete it.
		glDeleteTextures( 1, &Handle_ );
		Handle_ = 0;
	}
}

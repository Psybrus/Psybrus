/**************************************************************************
*
* File:		RsRenderBufferGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsRenderBufferGL.h"

//////////////////////////////////////////////////////////////////////////
// Ctor (colour)
RsRenderBufferGL::RsRenderBufferGL( eRsColourFormat Format, BcU32 Width, BcU32 Height )
{
	switch( Format )
	{
		case rsCF_A2R10G10B10:
			Format_ = GL_RGB10_A2;
			break;
		case rsCF_A8R8G8B8:
			Format_ = GL_RGBA8;
			break;
		case rsCF_X8R8G8B8:
			Format_ = GL_RGBA8;
			break;
		case rsCF_R16F:
			Format_ = GL_R16F;
			break;
		case rsCF_G16R16F:
			Format_ = GL_RG16F;
			break;
		case rsCF_A16B16G16R16F:
			Format_ = GL_RGBA16;
			break;
		case rsCF_R32F:
			Format_ = GL_R32F;
			break;
		case rsCF_G32R32F:
			Format_ = GL_RG32F;
			break;

		default:
			Format_ = GL_RGBA8; // Unsupported, fallback.
			break;

	}

	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// Ctor (depth/stencil)
RsRenderBufferGL::RsRenderBufferGL( eRsDepthStencilFormat Format, BcU32 Width, BcU32 Height )
{
	switch( Format )
	{
		case rsDSF_D16:
			Format_ = GL_DEPTH_COMPONENT16;
			break;
		case rsDSF_D32:
			Format_ = GL_DEPTH_COMPONENT32;
			break;
		case rsDSF_D24S8:
			Format_ = GL_DEPTH24_STENCIL8;
			break;
		case rsDSF_D32F:
			Format_ = GL_DEPTH_COMPONENT32F;
			break;

		default:
			Format_ = GL_DEPTH24_STENCIL8; // Unsupported, fallback.
			break;
	}
	
	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsRenderBufferGL::~RsRenderBufferGL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsRenderBufferGL::create()
{
	// Generate buffers.
	GLuint Handle;
	glGenRenderbuffers( 1, &Handle );
	setHandle( Handle );
	
	if( Handle != 0 )
	{
		update();
		
		// Destroy if there is a failure.
		if ( glGetError() != GL_NO_ERROR )
		{
			destroy();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsRenderBufferGL::update()
{
	GLuint Handle = getHandle< GLuint >();
	
	if( Handle != 0 )
	{
		// Bind render buffer.
		glBindRenderbuffer( GL_RENDERBUFFER, Handle );
		
		// Setup render buffer.
		glRenderbufferStorage( GL_RENDERBUFFER, Format_, Width_, Height_ );
		
		// Unbind render buffer.
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsRenderBufferGL::destroy()
{
	GLuint Handle = getHandle< GLuint >();
	
	if( Handle != 0 )
	{
		glDeleteRenderbuffers( 1, &Handle );
		setHandle< GLuint >( 0 );
	}
}


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

#include "System/Renderer/GL/RsRenderBufferGL.h"

//////////////////////////////////////////////////////////////////////////
// Ctor (colour)
RsRenderBufferGL::RsRenderBufferGL( RsContext* pContext, RsColourFormat Format, BcU32 Width, BcU32 Height ):
	RsResource( pContext )
{
	switch( Format )
	{
		case RsColourFormat::A2R10G10B10:
			Format_ = GL_RGB10_A2;
			break;
		case RsColourFormat::A8R8G8B8:
			Format_ = GL_RGBA8;
			break;
		case RsColourFormat::X8R8G8B8:
			Format_ = GL_RGBA8;
			break;
		case RsColourFormat::R16F:
			Format_ = GL_R16F;
			break;
		case RsColourFormat::G16R16F:
			Format_ = GL_RG16F;
			break;
		case RsColourFormat::A16B16G16R16F:
			Format_ = GL_RGBA16;
			break;
		case RsColourFormat::R32F:
			Format_ = GL_R32F;
			break;
		case RsColourFormat::G32R32F:
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
RsRenderBufferGL::RsRenderBufferGL( RsContext* pContext, RsDepthStencilFormat Format, BcU32 Width, BcU32 Height ):
	RsResource( pContext )
{
	switch( Format )
	{
		case RsDepthStencilFormat::D16:
			Format_ = GL_DEPTH_COMPONENT16;
			break;
		case RsDepthStencilFormat::D32:
			Format_ = GL_DEPTH_COMPONENT32;
			break;
		case RsDepthStencilFormat::D24S8:
			Format_ = GL_DEPTH24_STENCIL8;
			break;
		case RsDepthStencilFormat::D32F:
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
	GLuint Handle = 0;
	glGenRenderbuffers( 1, &Handle );
	RsGLCatchError();

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


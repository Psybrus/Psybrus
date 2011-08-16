/**************************************************************************
*
* File:		RsRenderTargetGL.cpp
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsRenderTargetGL.h"

#include "RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsRenderTargetGL::RsRenderTargetGL( eRsColourFormat ColourFormat, eRsDepthStencilFormat DepthStencilFormat, BcU32 Width, BcU32 Height, RsRenderBufferGL* pColourBuffer, RsRenderBufferGL* pDepthStencilBuffer, RsFrameBufferGL* pFrameBuffer, RsTextureGL* pTexture ):
	ColourFormat_( ColourFormat ),
	DepthStencilFormat_( DepthStencilFormat ),
	Width_( Width ),
	Height_( Height ),
	pColourBuffer_( pColourBuffer ),
	pDepthStencilBuffer_( pDepthStencilBuffer ),
	pFrameBuffer_( pFrameBuffer ),
	pTexture_( pTexture )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsRenderTargetGL::~RsRenderTargetGL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// bind
void RsRenderTargetGL::bind()
{
	if( this != NULL ) // HACK: Need to implement a default RT.
	{
		GLuint Handle = getHandle< GLuint >();
		glBindFramebuffer( GL_FRAMEBUFFER, Handle );
	}
	else
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// width
//virtual
BcU32 RsRenderTargetGL::width() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// height
//virtual
BcU32 RsRenderTargetGL::height() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
//colourFormat
//virtual
eRsColourFormat RsRenderTargetGL::colourFormat() const
{
	return ColourFormat_;
}

//////////////////////////////////////////////////////////////////////////
// depthStencilFormat
//virtual
eRsDepthStencilFormat RsRenderTargetGL::depthStencilFormat() const
{
	return DepthStencilFormat_;
}

//////////////////////////////////////////////////////////////////////////
// getTexture
//virtual
RsTexture* RsRenderTargetGL::getTexture()
{
	return pTexture_;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsRenderTargetGL::create()
{
	update();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsRenderTargetGL::update()
{
	GLuint ColourBufferHandle = pColourBuffer_->getHandle< GLuint >();
	GLuint DepthStencilBufferHandle = pDepthStencilBuffer_->getHandle< GLuint >();
	GLuint FrameBufferHandle = pFrameBuffer_->getHandle< GLuint >();
	GLuint TextureHandle = pTexture_->getHandle< GLuint >();

	// Bind framebuffer.
	glBindFramebuffer( GL_FRAMEBUFFER, FrameBufferHandle );
	
	// Setup attachments.
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER_EXT, ColourBufferHandle );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, DepthStencilBufferHandle );
	
	// Bind texture to framebuffer.
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandle, 0 );
	
	// Check the framebuffer is complete.
	GLenum Status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	BcAssertMsg( Status == GL_FRAMEBUFFER_COMPLETE, "RsRenderTargetGL: Framebuffer not complete." );

	// Unbind framebuffer.
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	// Set out handle to be the framebuffer handle.
	setHandle< GLuint >( FrameBufferHandle );
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsRenderTargetGL::destroy()
{
	RsCore::pImpl()->destroyResource( pColourBuffer_ );
	RsCore::pImpl()->destroyResource( pDepthStencilBuffer_ );
	RsCore::pImpl()->destroyResource( pFrameBuffer_ );
	RsCore::pImpl()->destroyResource( pTexture_ );
}


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

#include "System/Renderer/GL/RsRenderTargetGL.h"

#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsRenderTargetGL::RsRenderTargetGL( RsContext* pContext, const RsRenderTargetDesc& Desc, RsRenderBufferGL* pColourBuffer, RsRenderBufferGL* pDepthStencilBuffer, RsFrameBufferGL* pFrameBuffer, RsTextureGL* pTexture ):
	RsRenderTarget( pContext ),
	Desc_( Desc ),
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
	return Desc_.Width_;
}

//////////////////////////////////////////////////////////////////////////
// height
//virtual
BcU32 RsRenderTargetGL::height() const
{
	return Desc_.Height_;
}

//////////////////////////////////////////////////////////////////////////
//colourFormat
//virtual
eRsColourFormat RsRenderTargetGL::colourFormat( BcU32 Index ) const
{
	return Desc_.ColourFormats_[ Index ];
}

//////////////////////////////////////////////////////////////////////////
// depthStencilFormat
//virtual
eRsDepthStencilFormat RsRenderTargetGL::depthStencilFormat() const
{
	return Desc_.DepthStencilFormat_;
}

//////////////////////////////////////////////////////////////////////////
// getTexture
//virtual
RsTexture* RsRenderTargetGL::getTexture( BcU32 Index )
{
	BcAssert( Index == 0 );
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
	
	// NOTE: Don't need a render buffer for colour attachment. Remove this when we get to MRT.
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


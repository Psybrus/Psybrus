/**************************************************************************
*
* File:		RsCoreImplGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsCoreImplGL.h"

#include "RsFrameGL.h"

#include "RsTextureGL.h"
#include "RsRenderTargetGL.h"
#include "RsRenderBufferGL.h"
#include "RsFrameBufferGL.h"
#include "RsVertexBufferGL.h"
#include "RsIndexBufferGL.h"
#include "RsShaderGL.h"
#include "RsProgramGL.h"
#include "RsPrimitiveGL.h"
#include "RsStateBlockGL.h"

#include "SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( RsCoreImplGL );

//////////////////////////////////////////////////////////////////////////
// Ctor
RsCoreImplGL::RsCoreImplGL():
	pFrame_( NULL )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsCoreImplGL::~RsCoreImplGL()
{

}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void RsCoreImplGL::open()
{
	// NOTE: GL renderer uses SDL in this implementation.
	// TODO: Move into a higher level so this GL renderer
	//       can be used on any other platform.
	W_ = 1280;
	H_ = 720;
	
	// Setup default viewport.
	glViewport( 0, 0, W_, H_ );
		
	// Allocate a frame for rendering.
	pFrame_ = new RsFrameGL( NULL, W_, H_ );
		
	// Allocate a state block for rendering.
	pStateBlock_ = new RsStateBlockGL();
		
	// Clear.
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsCoreImplGL::update()
{
	// Set default state.
	pStateBlock_->setDefaultState();
	
	// Execute command buffer.
	CommandBuffer_.execute();

	glFlush();
	SDL_GL_SwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void RsCoreImplGL::close()
{
	// Free the state block.
	delete pStateBlock_;
	
	// Free the frame.
	delete pFrame_;

}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplGL::createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	RsTextureGL* pResource = new RsTextureGL( Width, Height, Levels, Format, pData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createRenderTarget
//virtual
RsRenderTarget*	RsCoreImplGL::createRenderTarget( BcU32 Width, BcU32 Height, eRsColourFormat ColourFormat, eRsDepthStencilFormat DepthStencilFormat )
{
	RsRenderBufferGL* pColourBuffer = new RsRenderBufferGL( ColourFormat, Width, Height );
	RsRenderBufferGL* pDepthStencilBuffer = new RsRenderBufferGL( DepthStencilFormat, Width, Height );
	RsFrameBufferGL* pFrameBuffer = new RsFrameBufferGL();
	RsTextureGL* pTexture = new RsTextureGL( Width, Height, 1, rsTF_RGBA8, NULL );

	createResource( pColourBuffer );
	createResource( pDepthStencilBuffer );
	createResource( pFrameBuffer );
	createResource( pTexture );

	// Create the render target.
	RsRenderTargetGL* pRenderTarget = new RsRenderTargetGL( ColourFormat, DepthStencilFormat, Width, Height, pColourBuffer, pDepthStencilBuffer, pFrameBuffer, pTexture );	
	createResource( pRenderTarget );
	
	return pRenderTarget;
}

//////////////////////////////////////////////////////////////////////////
// createVertexBuffer
//virtual 
RsVertexBuffer* RsCoreImplGL::createVertexBuffer( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData )
{
	RsVertexBufferGL* pResource = new RsVertexBufferGL( Descriptor, NoofVertices, pVertexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createIndexBuffer
//virtual 
RsIndexBuffer* RsCoreImplGL::createIndexBuffer( BcU32 NoofIndices, void* pIndexData )
{
	RsIndexBufferGL* pResource = new RsIndexBufferGL( NoofIndices, pIndexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createShader
//virtual
RsShader* RsCoreImplGL::createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize )
{
	RsShaderGL* pResource = new RsShaderGL( ShaderType, ShaderDataType, pShaderData, ShaderDataSize );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImplGL::createProgram( RsShader* pVertexShader, RsShader* pFragmentShader )
{
	RsProgramGL* pResource = new RsProgramGL( static_cast< RsShaderGL* >( pVertexShader ), static_cast< RsShaderGL* >( pFragmentShader ) );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createPrimitive
//virtual
RsPrimitive* RsCoreImplGL::createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer )
{
	RsPrimitiveGL* pResource = new RsPrimitiveGL( static_cast< RsVertexBufferGL* >( pVertexBuffer ), static_cast< RsIndexBufferGL* >( pIndexBuffer ) );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImplGL::destroyResource( RsResource* pResource )
{
	pResource->preDestroy();
	
	BcDelegateCall< void(*)() > DelegateCall( BcDelegate< void(*)() >::bind< RsResource, &RsResource::destroy >( pResource ) );
	CommandBuffer_.enqueue( DelegateCall );
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void RsCoreImplGL::updateResource( RsResource* pResource )
{
	BcDelegateCall< void(*)() > DelegateCall( BcDelegate< void(*)() >::bind< RsResource, &RsResource::update >( pResource ) );
	CommandBuffer_.enqueue( DelegateCall );
}

//////////////////////////////////////////////////////////////////////////
// createResource
void RsCoreImplGL::createResource( RsResource* pResource )
{
	BcDelegateCall< void(*)() > DelegateCall( BcDelegate< void(*)() >::bind< RsResource, &RsResource::create >( pResource ) );
	CommandBuffer_.enqueue( DelegateCall );
}

//////////////////////////////////////////////////////////////////////////
// allocateFrame
RsFrame* RsCoreImplGL::allocateFrame( BcHandle DeviceHandle, BcU32 Width, BcU32 Height )
{
	BcUnusedVar( DeviceHandle );
	BcUnusedVar( Width );
	BcUnusedVar( Height );
	
	return new RsFrameGL( NULL, W_, H_ );
}

//////////////////////////////////////////////////////////////////////////
// queueFrame
void RsCoreImplGL::queueFrame( RsFrame* pFrame )
{
	BcDelegateCall< void(*)() > DelegateCall( BcDelegate< void(*)() >::bind< RsFrameGL, &RsFrameGL::render >( (RsFrameGL*)pFrame ) );
	CommandBuffer_.enqueue( DelegateCall );
}

//////////////////////////////////////////////////////////////////////////
// getStateBlock
RsStateBlock* RsCoreImplGL::getStateBlock()
{
	return pStateBlock_;
}


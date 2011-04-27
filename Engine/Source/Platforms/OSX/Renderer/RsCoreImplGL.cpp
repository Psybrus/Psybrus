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
#include "RsVertexBufferGL.h"
#include "RsIndexBufferGL.h"
#include "RsShaderGL.h"
#include "RsProgramGL.h"
#include "RsPrimitiveGL.h"
#include "RsStateBlockGL.h"

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
	
	// Setup resolution.
	pScreenSurface_ = SDL_SetVideoMode( W_, H_, 32, SDL_HWSURFACE | SDL_OPENGLBLIT );
	
	if( pScreenSurface_ != NULL )
	{
		// Setup depth buffer.
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
		
		// Setup default viewport.
		glViewport( 0, 0, W_, H_ );
		
		// Allocate a frame for rendering.
		pFrame_ = new RsFrameGL( NULL, W_, H_ );
		
		// Allocate a state block for rendering.
		pStateBlock_ = new RsStateBlockGL();
	}
	else
	{
		stop();
	}
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

	// Free SDL surface.
	SDL_FreeSurface( pScreenSurface_ );
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
class DestroyResourceCommand: public BcCommand
{
public:
	DestroyResourceCommand( RsResource* pResource ):
	pResource_( pResource )
	{
	}
	
	void execute()
	{
		pResource_->destroy();
	}
	
private:
	RsResource* pResource_;
};

void RsCoreImplGL::destroyResource( RsResource* pResource )
{
	pResource->preDestroy();
	
	CommandBuffer_.push( new DestroyResourceCommand( pResource ) );
}

//////////////////////////////////////////////////////////////////////////
// updateResource
class UpdateResourceCommand: public BcCommand
{
public:
	UpdateResourceCommand( RsResource* pResource ):
	pResource_( pResource )
	{
	}
	
	void execute()
	{
		pResource_->update();
	}
	
private:
	RsResource* pResource_;
};

void RsCoreImplGL::updateResource( RsResource* pResource )
{
	CommandBuffer_.push( new UpdateResourceCommand( pResource ) );
}

//////////////////////////////////////////////////////////////////////////
// createResource
class CreateResourceCommand: public BcCommand
{
public:
	CreateResourceCommand( RsResource* pResource ):
		pResource_( pResource )
	{
	}
	
	void execute()
	{
		pResource_->create();
	}
	
private:
	RsResource* pResource_;
};

void RsCoreImplGL::createResource( RsResource* pResource )
{
	CommandBuffer_.push( new CreateResourceCommand( pResource ) );
}

//////////////////////////////////////////////////////////////////////////
// allocateFrame
RsFrame* RsCoreImplGL::allocateFrame( BcHandle DeviceHandle, BcU32 Width, BcU32 Height )
{
	BcUnusedVar( DeviceHandle );
	BcUnusedVar( Width );
	BcUnusedVar( Height );
	
	return pFrame_;	
}

//////////////////////////////////////////////////////////////////////////
// queueFrame
class RenderFrameCommand: public BcCommand
{
public:
	RenderFrameCommand( RsFrameGL* pFrame ):
		pFrame_( pFrame )
	{
	}
	
	void execute()
	{
		pFrame_->render();
	}
	
private:
	RsFrameGL* pFrame_;
};

void RsCoreImplGL::queueFrame( RsFrame* pFrame )
{
	CommandBuffer_.push( new RenderFrameCommand( static_cast< RsFrameGL* >( pFrame ) ) );	
}

//////////////////////////////////////////////////////////////////////////
// getStateBlock
RsStateBlock* RsCoreImplGL::getStateBlock()
{
	return pStateBlock_;
}


/**************************************************************************
*
* File:		RsCoreImplGLES.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsCoreImplGLES.h"

#include "RsFrameGLES.h"

#include "RsTextureGLES.h"
#include "RsVertexBufferGLES.h"
#include "RsIndexBufferGLES.h"
#include "RsShaderGLES.h"
#include "RsProgramGLES.h"
#include "RsPrimitiveGLES.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( RsCoreImplGLES );

//////////////////////////////////////////////////////////////////////////
// Ctor
RsCoreImplGLES::RsCoreImplGLES():
	pFrame_( NULL )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsCoreImplGLES::~RsCoreImplGLES()
{

}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void RsCoreImplGLES::open()
{
	BcU32 W = 320;
	BcU32 H = 480;
	
	// Allocate a frame for rendering.
	pFrame_ = new RsFrameGLES( NULL, W, H );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsCoreImplGLES::update()
{
	// Execute command buffer.
	CommandBuffer_.execute();
	
	//
	glFlush();
	glFinish();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void RsCoreImplGLES::close()
{
	// Free the frame.
	delete pFrame_;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplGLES::createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	RsTextureGLES* pResource = new RsTextureGLES( Width, Height, Levels, Format, pData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createVertexBuffer
//virtual 
RsVertexBuffer* RsCoreImplGLES::createVertexBuffer( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData )
{
	RsVertexBufferGLES* pResource = new RsVertexBufferGLES( Descriptor, NoofVertices, pVertexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createIndexBuffer
//virtual 
RsIndexBuffer* RsCoreImplGLES::createIndexBuffer( BcU32 NoofIndices, void* pIndexData )
{
	RsIndexBufferGLES* pResource = new RsIndexBufferGLES( NoofIndices, pIndexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createShader
//virtual
RsShader* RsCoreImplGLES::createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize )
{
	RsShaderGLES* pResource = new RsShaderGLES( ShaderType, ShaderDataType, pShaderData, ShaderDataSize );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImplGLES::createProgram( RsShader* pVertexShader, RsShader* pFragmentShader )
{
	RsProgramGLES* pResource = new RsProgramGLES( static_cast< RsShaderGLES* >( pVertexShader ), static_cast< RsShaderGLES* >( pFragmentShader ) );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createPrimitive
//virtual
RsPrimitive* RsCoreImplGLES::createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer )
{
	RsPrimitiveGLES* pResource = new RsPrimitiveGLES( static_cast< RsVertexBufferGLES* >( pVertexBuffer ), static_cast< RsIndexBufferGLES* >( pIndexBuffer ) );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createResource
class CreateResourceCommand: public BcCommand
{
public:
	CreateResourceCommand( RsGPUResourceGLES* pResource ):
		pResource_( pResource )
	{
	}
	
	void execute()
	{
		pResource_->create();
	}
	
private:
	RsGPUResourceGLES* pResource_;
};

void RsCoreImplGLES::createResource( RsGPUResourceGLES* pResource )
{
	CommandBuffer_.push( new CreateResourceCommand( pResource ) );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
class DestroyResourceCommand: public BcCommand
{
public:
	DestroyResourceCommand( RsGPUResourceGLES* pResource ):
		pResource_( pResource )
	{
	}
	
	void execute()
	{
		pResource_->destroy();
	}
	
private:
	RsGPUResourceGLES* pResource_;
};

void RsCoreImplGLES::destroyResource( RsGPUResourceGLES* pResource )
{
	CommandBuffer_.push( new DestroyResourceCommand( pResource ) );
}

//////////////////////////////////////////////////////////////////////////
// allocateFrame
RsFrame* RsCoreImplGLES::allocateFrame( BcHandle DeviceHandle, BcU32 Width, BcU32 Height )
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
	RenderFrameCommand( RsFrameGLES* pFrame ):
		pFrame_( pFrame )
	{
	}
	
	void execute()
	{
		pFrame_->render();
	}
	
private:
	RsFrameGLES* pFrame_;
};

void RsCoreImplGLES::queueFrame( RsFrame* pFrame )
{
	CommandBuffer_.push( new RenderFrameCommand( static_cast< RsFrameGLES* >( pFrame ) ) );	
}

/*
//////////////////////////////////////////////////////////////////////////
// setViewport
void RsCoreImplGLES::setViewport( const RsViewport* pViewport )
{
	// Set the viewport.
	glViewport( pViewport->x(), pViewport->y(), pViewport->width(), pViewport->height() );
	
	// Clear depth.
	if( pViewport->clearDepth() )
	{
		glClear( GL_DEPTH_BUFFER_BIT );
	}
}
*/


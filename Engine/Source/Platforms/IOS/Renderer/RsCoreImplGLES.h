/**************************************************************************
*
* File:		RsCoreImplGLES.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCOREIMPLGLES_H__
#define __RSCOREIMPLGLES_H__

#include "RsCore.h"
#include "RsGLES.h"

#include "BcMutex.h"
#include "BcCommandBuffer.h"

#include "RsGPUResourceGLES.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsFrameGLES;

//////////////////////////////////////////////////////////////////////////
// RsCoreImplGLES
class RsCoreImplGLES:
	public RsCore
{
public:
	RsCoreImplGLES();
	virtual ~RsCoreImplGLES();
	
public:
	virtual void			open();
	virtual void			update();
	virtual void			close();
	
public:
	virtual RsTexture*		createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData = NULL );
	virtual RsVertexBuffer*	createVertexBuffer( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData = NULL );
	virtual RsIndexBuffer*	createIndexBuffer( BcU32 NoofIndices, void* pIndexData = NULL );
	virtual RsShader*		createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual RsProgram*		createProgram( RsShader* pVertexShader, RsShader* pFragmentShader );
	virtual RsPrimitive*	createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer );

private:
	void					createResource( RsGPUResourceGLES* pResource );
	void					destroyResource( RsGPUResourceGLES* pResource );
	
public:
	RsFrame*				allocateFrame( BcHandle DeviceHandle, BcU32 Width, BcU32 Height );
	void					queueFrame( RsFrame* pFrame );
	
public:

public:
	// Platform specific interface.

protected:
	BcCommandBuffer			CommandBuffer_;
	RsFrameGLES*			pFrame_;

};

#endif



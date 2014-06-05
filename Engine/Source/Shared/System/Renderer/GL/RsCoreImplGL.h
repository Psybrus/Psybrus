/**************************************************************************
*
* File:		RsCoreImplGL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCOREIMPLGL_H__
#define __RSCOREIMPLGL_H__

#include <mutex>

#include "System/Renderer/RsCore.h"
#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsFrameGL;
class RsContextGL;

//////////////////////////////////////////////////////////////////////////
// RsCoreImplGL
class RsCoreImplGL:
	public RsCore
{
public:
	RsCoreImplGL();
	virtual ~RsCoreImplGL();
	
public:
	virtual void				open();
	void						open_threaded();
	virtual void				update();
	void						update_threaded();
	virtual void				close();
	void						close_threaded();
	
public:
	virtual RsContext*			getContext( OsClient* pClient );
	virtual void				destroyContext( OsClient* pClient );

	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Levels, RsTextureFormat Format, void* pData = NULL );
	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format, void* pData = NULL );
	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, RsTextureFormat Format, void* pData = NULL );
	virtual RsRenderTarget*		createRenderTarget( const RsRenderTargetDesc& Desc );
	virtual RsVertexDeclaration* createVertexDeclaration( const RsVertexDeclarationDesc& Desc );
	virtual RsVertexBuffer*		createVertexBuffer( const RsVertexBufferDesc& Desc, void* pVertexData = NULL );
	virtual RsIndexBuffer*		createIndexBuffer( const RsIndexBufferDesc& Desc, void* pIndexData = NULL );
	virtual RsUniformBuffer*	createUniformBuffer( const RsUniformBufferDesc& Desc, void* pBufferData = NULL );
	virtual RsShader*			createShader( RsShaderType ShaderType, RsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual RsProgram*			createProgram( BcU32 NoofShaders, RsShader** ppShaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes  );
	virtual void				destroyResource( RsResource* pResource );
	void						updateResource( RsResource* pResource );

private:
	void						createResource( RsResource* pResource );

public:
	RsFrame*					allocateFrame( RsContext* pContext );
	void						queueFrame( RsFrame* pFrame );
	void						queueFrame_threaded( RsFrameGL* pFrame );

public:
	// Platform specific interface.

protected:
	std::mutex					ResourceLock_;
	SysFence				RenderSyncFence_;

	typedef std::map< OsClient*, RsContextGL* > TContextMap;
	typedef TContextMap::iterator TContextMapIterator;

	TContextMap				ContextMap_;
		
};

#endif



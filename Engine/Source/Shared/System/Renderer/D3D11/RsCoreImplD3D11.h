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
#include "System/Renderer/D3D11/RsD3D11.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsFrameD3D11;
class RsContextD3D11;

//////////////////////////////////////////////////////////////////////////
// RsCoreImplD3D11
class RsCoreImplD3D11:
	public RsCore
{
public:
	RsCoreImplD3D11();
	virtual ~RsCoreImplD3D11();
	
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

	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Levels, eRsTextureFormat Format, void* pData = NULL );
	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData = NULL );
	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, eRsTextureFormat Format, void* pData = NULL );
	virtual RsRenderTarget*		createRenderTarget( const RsRenderTargetDesc& Desc );
	virtual RsVertexBuffer*		createVertexBuffer( const RsVertexBufferDesc& Desc, void* pVertexData = NULL );
	virtual RsIndexBuffer*		createIndexBuffer( const RsIndexBufferDesc& Desc, void* pIndexData = NULL );
	virtual RsUniformBuffer*	createUniformBuffer( const RsUniformBufferDesc& Desc, void* pBufferData = NULL );
	virtual RsShader*			createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual RsProgram*			createProgram( RsShader* pVertexShader, RsShader* pFragmentShader );
	virtual RsProgram*			createProgram( BcU32 NoofShaders, RsShader** ppShaders );
	virtual RsPrimitive*		createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer );
	virtual void				destroyResource( RsResource* pResource );
	void						updateResource( RsResource* pResource );

private:
	void						createResource( RsResource* pResource );

public:
	RsFrame*					allocateFrame( RsContext* pContext );
	void						queueFrame( RsFrame* pFrame );
	void						queueFrame_threaded( RsFrameD3D11* pFrame );

public:
	// Platform specific interface.

protected:
	SysFence					RenderSyncFence_;

	typedef std::map< OsClient*, RsContextD3D11* > TContextMap;
	typedef TContextMap::iterator TContextMapIterator;

	TContextMap				ContextMap_;
		
};

#endif



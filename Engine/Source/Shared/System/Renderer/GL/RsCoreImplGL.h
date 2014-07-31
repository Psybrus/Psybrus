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

	virtual RsTexture*			createTexture( const RsTextureDesc& Desc );
	virtual RsRenderTarget*		createRenderTarget( const RsRenderTargetDesc& Desc );
	virtual RsVertexDeclaration* createVertexDeclaration( const RsVertexDeclarationDesc& Desc );
	virtual RsBuffer*			createBuffer( const RsBufferDesc& Desc );
	virtual RsShader*			createShader( RsShaderType ShaderType, RsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual RsProgram*			createProgram( BcU32 NoofShaders, RsShader** ppShaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes  );
	virtual void				destroyResource( RsResource* pResource );
	virtual void				destroyResource( RsBuffer* Buffer );
	virtual void				destroyResource( RsTexture* Texture );
	void						updateResource( RsResource* pResource );

	//////////////////////////////////////////////////////////////////////
	// New interfaces.
	bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc );

	bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc );

private:
	struct UpdateBufferAsync
	{
		class RsBuffer* Buffer_;
		BcSize Offset_;
		BcSize Size_;
		RsResourceUpdateFlags Flags_;
		RsBufferUpdateFunc UpdateFunc_;
	};

	bool updateBuffer_threaded( 
		UpdateBufferAsync Cmd );

	bool destroyBuffer_threaded( 
		RsBuffer* Buffer );

	struct UpdateTextureAsync
	{
		class RsTexture* Texture_;
		RsTextureSlice Slice_;
		RsResourceUpdateFlags Flags_;
		RsTextureUpdateFunc UpdateFunc_;
	};

	bool updateTexture_threaded( 
		UpdateTextureAsync Cmd );

	bool destroyTexture_threaded( 
		RsTexture* Texture );
private:

	void						createResource( RsResource* pResource );

public:
	RsFrame*					allocateFrame( RsContext* pContext );
	void						queueFrame( RsFrame* pFrame );
	void						queueFrame_threaded( RsFrameGL* pFrame );

public:
	// Platform specific interface.

protected:
	SysFence				RenderSyncFence_;

	typedef std::map< OsClient*, RsContextGL* > TContextMap;
	typedef TContextMap::iterator TContextMapIterator;

	TContextMap				ContextMap_;
		
};

#endif



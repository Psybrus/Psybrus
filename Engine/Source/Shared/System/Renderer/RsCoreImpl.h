/**************************************************************************
*
* File:		RsCoreImpl.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RsCoreImpl_H__
#define __RsCoreImpl_H__

#include <mutex>

#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsFrameGL;
class RsContextGL;

//////////////////////////////////////////////////////////////////////////
// RsCoreImpl
class RsCoreImpl:
	public RsCore
{
public:
	RsCoreImpl();
	virtual ~RsCoreImpl();
	
public:
	void open();
	void open_threaded();
	void update();
	void update_threaded();
	void close();
	void close_threaded();
	
public:
	RsContext* getContext( OsClient* pClient );
	void destroyContext( OsClient* pClient );

	RsTexture* createTexture( const RsTextureDesc& Desc );
	RsVertexDeclaration* createVertexDeclaration( const RsVertexDeclarationDesc& Desc );
	RsBuffer* createBuffer( const RsBufferDesc& Desc );
	RsShader* createShader( RsShaderType ShaderType, RsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	RsProgram* createProgram( std::vector< RsShader* > Shaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes  );
	void destroyResource( RsResource* pResource );
	void destroyResource( RsBuffer* Buffer );
	void destroyResource( RsTexture* Texture );
	void updateResource( RsResource* pResource );

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

	void createResource( RsResource* pResource );

public:
	RsFrame* allocateFrame( RsContext* pContext );
	void queueFrame( RsFrame* pFrame );
	void queueFrame_threaded( RsFrame* pFrame );

public:
	// Platform specific interface.

protected:
	SysFence RenderSyncFence_;

	typedef std::map< OsClient*, RsContext* > TContextMap;
	typedef TContextMap::iterator TContextMapIterator;

	TContextMap ContextMap_;
		
};

#endif



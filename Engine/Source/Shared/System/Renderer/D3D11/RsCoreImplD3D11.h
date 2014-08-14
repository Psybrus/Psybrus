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

	virtual RsTexture*			createTexture( const RsTextureDesc& Desc );
	virtual RsVertexDeclaration* createVertexDeclaration( const RsVertexDeclarationDesc& Desc );
	virtual RsBuffer*			createBuffer( const RsBufferDesc& Desc );
	virtual RsShader*			createShader( RsShaderType ShaderType, RsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize );
	virtual RsProgram*			createProgram( std::vector< RsShader* > Shaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes  );
	virtual void				destroyResource( RsResource* pResource );
	virtual void				destroyResource( RsBuffer* Buffer );
	virtual void				destroyResource( RsTexture* Texture );
	void						updateResource( RsResource* pResource );

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



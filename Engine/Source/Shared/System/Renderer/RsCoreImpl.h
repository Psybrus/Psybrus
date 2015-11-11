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
	REFLECTION_DECLARE_DERIVED( RsCoreImpl, RsCore );

public:
	RsCoreImpl();
	virtual ~RsCoreImpl();
	
public:
	void open() override;
	void update() override;
	void close() override;
	
public:
	RsContext* getContext( OsClient* pClient ) override;
	void destroyContext( OsClient* pClient ) override;

	RsRenderStateUPtr createRenderState( 
		const RsRenderStateDesc& Desc ) override;

	RsSamplerStateUPtr createSamplerState( 
		const RsSamplerStateDesc& Desc ) override;

	RsFrameBufferUPtr createFrameBuffer( 
		const RsFrameBufferDesc& Desc ) override;

	RsTextureUPtr createTexture( 
		const RsTextureDesc& Desc ) override;

	RsVertexDeclarationUPtr createVertexDeclaration( 
		const RsVertexDeclarationDesc& Desc ) override;
	
	RsBufferUPtr createBuffer( 
		const RsBufferDesc& Desc ) override;
	
	RsShaderUPtr createShader( 
		const RsShaderDesc& Desc, 
		void* pShaderData, BcU32 ShaderDataSize,
		const std::string& DebugName ) override;
	
	RsProgramUPtr createProgram( 
		std::vector< RsShader* > Shaders, 
		RsProgramVertexAttributeList VertexAttributes,
		RsProgramUniformList UniformList,
		RsProgramUniformBlockList UniformBlockList,
		const std::string& DebugName ) override;
	
	RsProgramBindingUPtr createProgramBinding( 
		RsProgram* Program,
		const RsProgramBindingDesc& ProgramBindingDesc,
		const std::string& DebugName ) override;

	RsGeometryBindingUPtr createGeometryBinding( 
		const RsGeometryBindingDesc& GeometryBindingDesc,
		const std::string& DebugName ) override;


	void destroyResource( 
		RsResource* pResource ) override;
	
	void destroyResource( 
		RsRenderState* RenderState ) override;

	void destroyResource( 
		RsSamplerState* SamplerState ) override;

	void destroyResource( 
		RsBuffer* Buffer ) override;
	
	void destroyResource( 
		RsTexture* Texture ) override;
	
	void destroyResource( 
		RsFrameBuffer* FrameBuffer ) override;

	void destroyResource( 
		RsShader* Shader ) override;

	void destroyResource( 
		RsProgram* Program ) override;

	void destroyResource( 
		RsProgramBinding* ProgramBinding ) override;

	void destroyResource( 
		RsGeometryBinding* GeometryBinding ) override;

	void destroyResource( 
		RsVertexDeclaration* VertexDeclaration ) override;

	void updateResource( 
		RsResource* pResource ) override;

	//////////////////////////////////////////////////////////////////////
	// New interfaces.
	bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) override;

	bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc ) override;

private:
	struct UpdateBufferAsync
	{
		class RsBuffer* Buffer_;
		BcSize Offset_;
		BcSize Size_;
		RsResourceUpdateFlags Flags_;
		RsBufferUpdateFunc UpdateFunc_;
	};

	struct UpdateTextureAsync
	{
		class RsTexture* Texture_;
		RsTextureSlice Slice_;
		RsResourceUpdateFlags Flags_;
		RsTextureUpdateFunc UpdateFunc_;
	};

private:
	void createResource( RsResource* pResource );

public:
	RsFrame* allocateFrame( RsContext* pContext ) override;
	void queueFrame( RsFrame* pFrame ) override;
	BcF64 getFrameTime() const override;

public:
	// Platform specific interface.

protected:
	SysFence RenderSyncFence_;

	typedef std::map< OsClient*, RsContext* > TContextMap;
	typedef TContextMap::iterator TContextMapIterator;

	TContextMap ContextMap_;

	std::vector< std::function< void() > > ResourceDeletionList_;

	BcF64 FrameTime_;
	
};

#endif



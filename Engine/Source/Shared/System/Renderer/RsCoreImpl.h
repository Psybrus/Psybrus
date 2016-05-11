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

#include "System/Renderer/RsCore.h"

#include <mutex>
#include <unordered_set>

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
		const RsRenderStateDesc& Desc, const BcChar* DebugName ) override;

	RsSamplerStateUPtr createSamplerState( 
		const RsSamplerStateDesc& Desc, const BcChar* DebugName ) override;

	RsFrameBufferUPtr createFrameBuffer( 
		const RsFrameBufferDesc& Desc, const BcChar* DebugName ) override;

	RsTextureUPtr createTexture( 
		const RsTextureDesc& Desc, const BcChar* DebugName ) override;

	RsVertexDeclarationUPtr createVertexDeclaration( 
		const RsVertexDeclarationDesc& Desc, const BcChar* DebugName ) override;
	
	RsBufferUPtr createBuffer( 
		const RsBufferDesc& Desc,
		const BcChar* DebugName ) override;
	
	RsShaderUPtr createShader( 
		const RsShaderDesc& Desc, 
		void* pShaderData, BcU32 ShaderDataSize,
		const BcChar* DebugName ) override;
	
	RsProgramUPtr createProgram( 
		std::vector< RsShader* > Shaders, 
		RsProgramVertexAttributeList VertexAttributes,
		RsProgramParameterList ParameterList,
		const BcChar* DebugName ) override;
	
	RsProgramBindingUPtr createProgramBinding( 
		RsProgram* Program,
		const RsProgramBindingDesc& ProgramBindingDesc,
		const BcChar* DebugName ) override;

	RsGeometryBindingUPtr createGeometryBinding( 
		const RsGeometryBindingDesc& GeometryBindingDesc,
		const BcChar* DebugName ) override;

	RsQueryHeapUPtr createQueryHeap( 
		const RsQueryHeapDesc& Desc, 
		const BcChar* DebugName ) override;

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

	void destroyResource( 
		RsQueryHeap* QueryHeap ) override;

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

#if !PSY_PRODUCTION
	std::mutex AliveLock_;
	std::unordered_set< RsFrameBuffer* > AliveFrameBuffers_;
	std::unordered_set< RsProgram* > AlivePrograms_;
	std::unordered_set< RsGeometryBinding* > AliveGeometryBindings_;
	std::unordered_set< RsProgramBinding* > AliveProgramBindings_;

#endif

	BcF64 FrameTime_;
};

#endif



/**************************************************************************
*
* File:		RsCore.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCORE_H__
#define __RSCORE_H__

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsViewport.h"
#include "System/Renderer/RsFrame.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsComputeInterface.h"
#include "System/Renderer/RsContext.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsGeometryBinding.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsProgramBinding.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsUniquePointers.h"
#include "System/Renderer/RsVertexDeclaration.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsTexture;
class RsFrame;
class RsRenderTarget;

//////////////////////////////////////////////////////////////////////////
/**	\class RsCore
*	\brief Render System Core 
*
*	This provides the base interface that all renderers must adhere to.<br/><br/>
*	There are 2 types of call exposed:<br/>
*	* GAME - Functions safe to be called from the game thread.
*	* RENDER - Functions that may call the low-level API, or affect state.
*/
class RsCore:
	public BcGlobal< RsCore >,
	public SysSystem
{
public:
	static size_t JOB_QUEUE_ID;

	REFLECTION_DECLARE_DERIVED( RsCore, SysSystem );

public:
	RsCore(){};
	virtual ~RsCore(){};
	
public:

	/**
	 *	Get rendering context for client.
	 *	@param pClient Client to get a context for.
	 */
	virtual RsContext* getContext( 
		OsClient* pClient ) = 0;
	
	/**
	 *	Destroy rendering context for client.
	 *	@param pClient Client to get a context for.
	 */
	virtual void destroyContext( 
		OsClient* pClient ) = 0;

	/**
	 *	Create a render state.
	 *	@param Desc descriptor.
	 *	@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsRenderStateUPtr createRenderState( 
		const RsRenderStateDesc& Desc, 
		const BcChar* DebugName ) = 0;

	/**
	 *	Create a sampler state.
	 *	@param Desc descriptor.
	 * @param DebugName Name to use in debug message + assertions.
	 */
	virtual RsSamplerStateUPtr createSamplerState( 
		const RsSamplerStateDesc& Desc, 
		const BcChar* DebugName ) = 0;

	/**
	 *	Create frame buffer.
	 *	@param Desc descriptor.
	 *	@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsFrameBufferUPtr createFrameBuffer( 
		const RsFrameBufferDesc& Desc, 
		const BcChar* DebugName ) = 0;

	/**
	 *	Create a texture.
	 *	@param Desc descriptor.
	 *	@param pData Texture data.
	 *	@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsTextureUPtr createTexture( 
		const RsTextureDesc& Desc, 
		const BcChar* DebugName ) = 0;

	/**
	 *	Create a vertex declaration.
	 *	@param Desc Descriptor object.
	 *	@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsVertexDeclarationUPtr createVertexDeclaration( 
		const RsVertexDeclarationDesc& Desc, 
		const BcChar* DebugName ) = 0;

	/*
	 *	Create a buffer.
	 *	@param Desc Buffer descriptor
	 *	@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsBufferUPtr createBuffer( 
		const RsBufferDesc& Desc, 
		const BcChar* DebugName ) = 0;
	
	/**
	 *	Create shader.
	 *	@param Desc Shader descriptor.
	 *	@param pShaderData Shader data.
	 *	@param ShaderDataSize Shader data size.
		@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsShaderUPtr createShader( 
		const RsShaderDesc& Desc, 
		void* pShaderData, BcU32 ShaderDataSize,
		const BcChar* DebugName ) = 0;

	/**
	 *	Create program.
	 *	@param Shaders Array of shaders to use for program.
	 *	@param VertexAttributes Vertex attributes for program.
	 *	@param UniformList Uniforms for program.
	 *	@param UniformBlockList Uniform blocks for program.
	 *	@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsProgramUPtr createProgram( 
		std::vector< RsShader* > Shaders, 
		RsProgramVertexAttributeList VertexAttributes,
		RsProgramUniformList UniformList,
		RsProgramUniformBlockList UniformBlockList,
		const BcChar* DebugName ) = 0;

	/**
	 * Create program binding.
	 * @param Program to create binding for.
	 * @param ProgramBindingDesc Program binding descriptor.
	 * @param DebugName Name used for debugging creation.
	 */
	virtual RsProgramBindingUPtr createProgramBinding( 
		RsProgram* Program,
		const RsProgramBindingDesc& ProgramBindingDesc,
		const BcChar* DebugName ) = 0;

	/**
	 *	Create geometry binding.
	 *	@param GeometryBindingDesc Geometry binding descriptor.
	 *	@param DebugName Name used for debugging creation.
	 *	@param DebugName Name to use in debug message + assertions.
	 */
	virtual RsGeometryBindingUPtr createGeometryBinding( 
		const RsGeometryBindingDesc& GeometryBindingDesc,
		const BcChar* DebugName ) = 0;

	/**
	 *	Update resource. Work done on render thread.
	 *	@param pResource Resource to update.
	 */
	virtual void updateResource( 
		RsResource* pResource ) = 0;

	/**
	 *	Destroy resource. Work done on render thread.
	 *	@param pResource Resource to destroy.
	 */
	virtual void destroyResource( 
		RsResource* pResource ) = 0;
	virtual void destroyResource( 
		RsRenderState* RenderState ) = 0;
	virtual void destroyResource( 
		RsSamplerState* SamplerState ) = 0;
	virtual void destroyResource( 
		RsBuffer* Buffer ) = 0;
	virtual void destroyResource( 
		RsTexture* Texture ) = 0;
	virtual void destroyResource( 
		RsFrameBuffer* Texture ) = 0;
	virtual void destroyResource( 
		RsShader* Shader ) = 0;
	virtual void destroyResource( 
		RsProgram* Program ) = 0;
	virtual void destroyResource( 
		RsProgramBinding* ProgramBinding ) = 0;
	virtual void destroyResource( 
		RsGeometryBinding* GeometryBinding ) = 0;
	virtual void destroyResource( 
		RsVertexDeclaration* VertexDeclaration ) = 0;

	//////////////////////////////////////////////////////////////////////
	// New interfaces.
	
	/**
	 *	Update buffer.
	 *	@param Buffer Pointer to buffer.
	 *	@param Offset Offset in vertex buffer in bytes.
	 *	@param Size Size to update in bytes. If 0, whole size of buffer is assumed.
	 *	@param Flags Resource update flags.
	 *	@param UpdateFunc Function to call for update.
	 */
	virtual bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) = 0;

	/**
	 *	Update texture.
	 */
	virtual bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc ) = 0;	
public:
	/**
	 *	Allocate a frame for rendering.
	 *	GAME FUCTION: Called to get a frame prior to queuing up render objects.
	 *	@param pContext Rendering context to allocate frame for use with.
	 */
	virtual RsFrame* allocateFrame( RsContext* pContext ) = 0;

	/**
	 *	Queue a frame for rendering.\n
	 *	GAME FUNCTION: Called from game thread to queue frame to be rendered.
	 */
	virtual void queueFrame( RsFrame* pFrame ) = 0;

	/**
	 *	Get frame time.
	 *	Time spent on last frame.
	 */
	virtual BcF64 getFrameTime() const = 0;
};

#endif


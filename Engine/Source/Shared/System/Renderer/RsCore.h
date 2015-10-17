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
#include "System/Renderer/RsContext.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsVertexDeclaration.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsTexture;
class RsFrame;
class RsRenderTarget;

//////////////////////////////////////////////////////////////////////////
// Resource deletion.
class RsResourceDeleters
{
public:
	void operator()( class RsBuffer* Resource );
	void operator()( class RsContext* Resource );
	void operator()( class RsFrameBuffer* Resource );
	void operator()( class RsProgram* Resource );
	void operator()( class RsRenderState* Resource );
	void operator()( class RsSamplerState* Resource );
	void operator()( class RsShader* Resource );
	void operator()( class RsTexture* Resource );
	void operator()( class RsVertexDeclaration* Resource );
};

typedef std::unique_ptr< class RsBuffer, RsResourceDeleters > RsBufferUPtr;
typedef std::unique_ptr< class RsContext, RsResourceDeleters > RsContextUPtr;
typedef std::unique_ptr< class RsFrameBuffer, RsResourceDeleters > RsFrameBufferUPtr;
typedef std::unique_ptr< class RsProgram, RsResourceDeleters > RsProgramUPtr;
typedef std::unique_ptr< class RsRenderState, RsResourceDeleters > RsRenderStateUPtr;
typedef std::unique_ptr< class RsSamplerState, RsResourceDeleters > RsSamplerStateUPtr;
typedef std::unique_ptr< class RsShader, RsResourceDeleters > RsShaderUPtr;
typedef std::unique_ptr< class RsTexture, RsResourceDeleters > RsTextureUPtr;
typedef std::unique_ptr< class RsVertexDeclaration, RsResourceDeleters > RsVertexDeclarationUPtr;

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
	 */
	virtual RsRenderStateUPtr createRenderState( 
		const RsRenderStateDesc& Desc ) = 0;

	/**
	 *	Create a sampler state.
	 *	@param Desc descriptor.
	 */
	virtual RsSamplerStateUPtr createSamplerState( 
		const RsSamplerStateDesc& Desc ) = 0;

	/**
	 * Create frame buffer.
	 * @param Desc descriptor.
	 */
	virtual RsFrameBufferUPtr createFrameBuffer( 
		const RsFrameBufferDesc& Desc ) = 0;

	/**
	 *	Create a texture.
	 *	@param Desc descriptor.
	 *	@param pData Texture data.
	 */
	virtual RsTexture* createTexture( 
		const RsTextureDesc& Desc ) = 0;

	/**
	 *	Create a vertex declaration.
	 *	@param Desc Descriptor object.
	 */
	virtual RsVertexDeclaration* createVertexDeclaration( 
		const RsVertexDeclarationDesc& Desc ) = 0;

	/*
	 * Create a buffer.
	 * @param Desc Buffer descriptor
	 */
	virtual RsBuffer* createBuffer( 
		const RsBufferDesc& Desc ) = 0;
	
	/**
	 * Create shader.
	 * @param Desc Shader descriptor.
	 * @param pShaderData Shader data.
	 * @param ShaderDataSize Shader data size.
	 * @param DebugName Name used for debugging creation.
	 */
	virtual RsShader* createShader( 
		const RsShaderDesc& Desc, 
		void* pShaderData, BcU32 ShaderDataSize,
		const std::string& DebugName ) = 0;

	/**
	 * Create program.
	 * @param Shaders Array of shaders to use for program.
	 * @param VertexAttributes Vertex attributes for program.
	 * @param UniformList Uniforms for program.
	 * @param UniformBlockList Uniform blocks for program.
	 * @param DebugName Name used for debugging creation.
	 */
	virtual RsProgram* createProgram( 
		std::vector< RsShader* > Shaders, 
		RsProgramVertexAttributeList VertexAttributes,
		RsProgramUniformList UniformList,
		RsProgramUniformBlockList UniformBlockList,
		const std::string& DebugName ) = 0;

	/**
	 * Update resource. Work done on render thread.
	 * @param pResource Resource to update.
	 */
	virtual void updateResource( 
		RsResource* pResource ) = 0;

	/**
	 * Destroy resource. Work done on render thread.
	 * @param pResource Resource to destroy.
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
		RsVertexDeclaration* VertexDeclaration ) = 0;

	//////////////////////////////////////////////////////////////////////
	// New interfaces.
	
	/**
	 * Update buffer.
	 * @param Buffer Pointer to buffer.
	 * @param Offset Offset in vertex buffer in bytes.
	 * @param Size Size to update in bytes. If 0, whole size of buffer is assumed.
	 * @param Flags Resource update flags.
	 * @param UpdateFunc Function to call for update.
	 */
	virtual bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) = 0;

	/**
	 * Update texture.
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
	virtual RsFrame*			allocateFrame( RsContext* pContext ) = 0;

	/**
	*	Queue a frame for rendering.\n
	*	GAME FUNCTION: Called from game thread to queue frame to be rendered.
	*/
	virtual void				queueFrame( RsFrame* pFrame ) = 0;

	/**
	*	Get frame time.
	*	Time spent on last frame.
	*/
	virtual BcF32				getFrameTime() const = 0;
};

#endif


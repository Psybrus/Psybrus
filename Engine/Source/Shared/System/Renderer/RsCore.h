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
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsVertex.h"
#include "System/Renderer/RsViewport.h"
#include "System/Renderer/RsProjector.h"
#include "System/Renderer/RsLight.h"
#include "System/Renderer/RsFrame.h"

#include "System/Renderer/RsContext.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsRenderTarget.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsVertexDeclaration.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsBuffer.h"

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
	static BcU32 JOB_QUEUE_ID;

public:
	virtual ~RsCore(){};
	
public:

	/**
	 *	Get rendering context for client.
	 *	@param pClient Client to get a context for.
	 */
	virtual RsContext*			getContext( OsClient* pClient ) = 0;
	
	/**
	 *	Destroy rendering context for client.
	 *	@param pClient Client to get a context for.
	 */
	virtual void				destroyContext( OsClient* pClient ) = 0;

	/**
	 *	Create a texture.
	 *	@param Width Width.
	 *	@param Levels Mipmap Levels.
	 *	@param Format Texture format.
	 *	@param pData Texture data.
	 */
	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Levels, RsTextureFormat Format, void* pData = NULL ) = 0;

	/**
	 *	Create a texture.
	 *	@param Width Width.
	 *	@param Height Height.
	 *	@param Levels Mipmap Levels.
	 *	@param Format Texture format.
	 *	@param pData Texture data.
	 */
	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format, void* pData = NULL ) = 0;

	/**
	 *	Create a texture.
	 *	@param Width Width.
	 *	@param Height Height.
	 *	@param Depth Depth.
	 *	@param Levels Mipmap Levels.
	 *	@param Format Texture format.
	 *	@param pData Texture data.
	 */
	virtual RsTexture*			createTexture( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, RsTextureFormat Format, void* pData = NULL ) = 0;

	/**
	 *	Create a render target.
	 *	@param Desc Descriptor object.
	 */
	virtual RsRenderTarget*		createRenderTarget( const RsRenderTargetDesc& Desc ) = 0;

	/**
	 *	Create a vertex declaration.
	 *	@param Desc Descriptor object.
	 */
	virtual RsVertexDeclaration* createVertexDeclaration( const RsVertexDeclarationDesc& Desc ) = 0;

	/*
	 * Create a buffer.
	 * @param Desc Buffer descriptor
	 */
	virtual RsBuffer*			createBuffer( const RsBufferDesc& Desc ) = 0;
	
	/**
	 * Create shader.
	 * @param ShaderType Shader type.
	 * @param ShaderDataType Shader data type.
	 * @param pShaderData Shader data.
	 * @param ShaderDataSize Shader data size.
	 */
	virtual RsShader*			createShader( RsShaderType ShaderType, RsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize ) = 0;

	/**
	 * Create program.
	 * @param NoofShaders Number of shaders.
	 * @param ppShaders Array of shaders to use for program.
	 * @param NoofVertexAttributes Number of vertex attributes.
	 * @param pVertexAttributes Vertex attributes.
	 */
	virtual RsProgram*			createProgram( BcU32 NoofShaders, RsShader** ppShaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes  ) = 0;

	/**
	 * Update resource. Work done on render thread.
	 * @param pResource Resource to update.
	 */
	virtual void				updateResource( RsResource* pResource ) = 0;

	/**
	 * Destroy resource. Work done on render thread.
	 * @param pResource Resource to destroy.
	 */
	virtual void				destroyResource( RsResource* pResource ) = 0;
	virtual void				destroyResource( RsBuffer* Buffer ) = 0;

	//////////////////////////////////////////////////////////////////////
	// New interfaces.
	
	/**
	 * Update buffer.
	 */
	virtual bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsBufferUpdateFlags Flags,
		RsUpdateBufferFunc UpdateFunc ) = 0;
	
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
};

#endif


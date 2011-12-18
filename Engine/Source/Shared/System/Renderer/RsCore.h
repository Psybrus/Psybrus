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

#include "BcGlobal.h"
#include "SysSystem.h"

#include "RsTypes.h"
#include "RsVertex.h"
#include "RsViewport.h"
#include "RsProjector.h"
#include "RsLight.h"
#include "RsFrame.h"

#include "RsTexture.h"
#include "RsRenderTarget.h"
#include "RsShader.h"
#include "RsProgram.h"
#include "RsVertexBuffer.h"
#include "RsIndexBuffer.h"
#include "RsPrimitive.h"
#include "RsStateBlock.h"

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
	static const BcU32 WORKER_MASK = 0x2;

public:
	virtual ~RsCore(){};
	
public:
	/**
	 *	Create a texture.
	 *	@param Width Width.
	 *	@param Height Height.
	 *	@param Levels Mipmap Levels.
	 *	@param Format Texture format.
	 *	@param pData Texture data.
	 */
	virtual RsTexture*		createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData = NULL ) = 0;

	/**
	 *	Create a render target.
	 *	@param Width Width.
	 *	@param Height Height.
	 *  @param ColourFormat Colour format.
	 *  @param DepthStencilFormat Depth/stencil format.
	 */
	virtual RsRenderTarget*	createRenderTarget( BcU32 Width, BcU32 Height, eRsColourFormat ColourFormat, eRsDepthStencilFormat DepthStencilFormat ) = 0;

	/*
	 * Create a vertex buffer.
	 * @param Descriptor Vertex descriptor flags.
	 * @param NoofVertices Number of vertices.
	 * @param pVertexData Pointer to vertex data, NULL to create own.
	 */
	virtual RsVertexBuffer*	createVertexBuffer( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData = NULL ) = 0;
	
	/**
	 * Create index buffer.
	 * @param NoofIndices Number of indices.
	 * @param pIndexData Pointer to index data, NULL to create own.
	 */
	virtual RsIndexBuffer*	createIndexBuffer( BcU32 NoofIndices, void* pIndexData = NULL ) = 0;
	
	/**
	 * Create shader.
	 * @param ShaderType Shader type.
	 * @param ShaderDataType Shader data type.
	 * @param pShaderData Shader data.
	 * @param ShaderDataSize Shader data size.
	 */
	virtual RsShader*		createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize ) = 0;

	/**
	 * Create program.
	 * @param pVertexShader Vertex shader.
	 * @param pFragmentShader Fragment shader.
	 */
	virtual RsProgram*		createProgram( RsShader* pVertexShader, RsShader* pFragmentShader ) = 0;

	/**
	 * Create primitive.
	 * @param pVertexBuffer Vertex buffer.
	 * @param pIndexBuffer Index buffer.
	 */
	virtual RsPrimitive*	createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer ) = 0;

	/**
	 * Update resource. Work done on render thread.
	 * @param pResource Resource to update.
	 */
	virtual void			updateResource( RsResource* pResource ) = 0;

	/**
	 * Destroy resource. Work done on render thread.
	 * @param pResource Resource to destroy.
	 */
	virtual void			destroyResource( RsResource* pResource ) = 0;
	
public:
	/**
	*	Allocate a frame for rendering.
	*	GAME FUCTION: Called to get a frame prior to queuing up render objects.
	*	@param DeviceHandle Handle to device to render to. 0 for default.
	*/
	virtual RsFrame*		allocateFrame( BcHandle DeviceHandle = 0, BcU32 Width = BcErrorCode, BcU32 Height = BcErrorCode ) = 0;

	/**
	*	Queue a frame for rendering.\n
	*	GAME FUNCTION: Called from game thread to queue frame to be rendered.
	*/
	virtual void			queueFrame( RsFrame* pFrame ) = 0;

	/**
	 *	Get render state block.
	 */
	virtual RsStateBlock*	getStateBlock() = 0;
};

#endif


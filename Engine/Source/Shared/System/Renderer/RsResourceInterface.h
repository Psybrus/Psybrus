/**************************************************************************
*
* File:		RsResourceInterface.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSRESOURCEINTERFACE_H__
#define __RSRESOURCEINTERFACE_H__

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsResourceInterface
class RsResourceInterface
{
public:
	virtual ~RsResourceInterface(){};

	/**
	 * Create render state.
	 * @param RenderState Render state to create from.
	 */
	virtual bool createRenderState(
		class RsRenderState* RenderState ) = 0;

	/**
	 * Destroy render state.
	 * @param RenderState Render state to destroy.
	 */
	virtual bool destroyRenderState( 
		class RsRenderState* RenderState ) = 0;

	/**
	 * Create sampler state.
	 * @param SamplerState Sampler state to create from.
	 */
	virtual bool createSamplerState(
		class RsSamplerState* SamplerState ) = 0;

	/**
	 * Destroy sampler state.
	 * @param SamplerState Render state to destroy.
	 */
	virtual bool destroySamplerState( 
		class RsSamplerState* SamplerState ) = 0;

	/**
	 * Create frame buffer.
	 * @param FrameBuffer Frame buffer to create from.
	 */
	virtual bool createFrameBuffer( 
		class RsFrameBuffer* FrameBuffer ) = 0;

	/**
	 * Destroy frame buffer.
	 * @param FrameBuffer Frane buffer to destroy.
	 */
	virtual bool destroyFrameBuffer( 
		class RsFrameBuffer* FrameBuffer ) = 0;

	/**
	 * Create buffer.
	 * @param Buffer Buffer to create from.
	 */
	virtual bool createBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Destroy buffer.
	 * @param Buffer Buffer to destroy.
	 */
	virtual bool destroyBuffer( 
		class RsBuffer* Buffer ) = 0;

	/**
	 * Update buffer.
	 * @param Buffer to update.
	 * @param Offset to update.
	 * @param Size to update.
	 * @param Flags for update.
	 * @param UpdateFunc to call.
	 */
	virtual bool updateBuffer( 
		class RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) = 0;

	/**
	 * Create texture.
	 * @param Texture Texture to create from.
	 */
	virtual bool createTexture( 
		class RsTexture* Texture ) = 0;

	/**
	 * Destroy texture.
	 * @param Texture Texture to destroy.
	 */
	virtual bool destroyTexture( 
		class RsTexture* Texture ) = 0;

	/**
	 * Update texture
	 * @param Texture to update.
	 */
	virtual bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc ) = 0;

	/**
	 * Create shader.
	 * @param Shader Shader to create from.
	 */
	virtual bool createShader(
		class RsShader* Shader ) = 0;

	/**
	 * Destroy shader.
	 * @param Shader Shader to destroy.
	 */
	virtual bool destroyShader(
		class RsShader* Shader ) = 0;

	/**
	 * Create program.
	 * @param Program Program to create from.
	 */
	virtual bool createProgram(
		class RsProgram* Program ) = 0;

	/**
	 * Destroy program.
	 * @param Program Program to destory.
	 */
	virtual bool destroyProgram(
		class RsProgram* Program ) = 0;

	/**
	 * Create program binding.
	 * @param ProgramBinding Program binding to create from.
	 */
	virtual bool createProgramBinding(
		class RsProgramBinding* ProgramBinding ) = 0;

	/**
	 * Destroy program binding.
	 * @param ProgramBinding Program binding to destroy.
	 */
	virtual bool destroyProgramBinding( 
		class RsProgramBinding* ProgramBinding ) = 0;

	/**
	 * Create geometry binding.
	 * @param GeometryBinding GeometryBinding to create from.
	 */
	virtual bool createGeometryBinding( 
		class RsGeometryBinding* GeometryBinding ) = 0;

	/**
	 * Create geometry binding.
	 * @param GeometryBinding GeometryBinding to create from.
	 */
	virtual bool destroyGeometryBinding( 
		class RsGeometryBinding* GeometryBinding ) = 0;

	/**
	 * Create vertex declaration.
	 * @param VertexDeclaration VertexDeclaration to create from.
	 */
	virtual bool createVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration ) = 0;

	/**
	 * Destroy VertexDeclaration.
	 * @param VertexDeclaration VertexDeclaration to destory.
	 */
	virtual bool destroyVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration  ) = 0;

};

#endif

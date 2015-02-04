/**************************************************************************
*
* File:		RsDrawInterface.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSDRAWINTERFACE_H__
#define __RSDRAWINTERFACE_H__

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsDrawInterface
class RsDrawInterface
{
public:
	virtual ~RsDrawInterface(){};

	/**
	 * Set default state.
	 */
	virtual void setDefaultState() = 0;
	
	/**
	 * Invalidate render state.
	 */
	virtual void invalidateRenderState() = 0;
	
	/**
	 * Invalidate texture state.
	 */
	virtual void invalidateTextureState() = 0;

	/**
	 * Set render state.
	 */
	virtual void setRenderState( RsRenderState* RenderState ) = 0;
	
	/**
	 * Set sampler state.
	 */
	virtual void setSamplerState( BcU32 Slot, RsSamplerState* SamplerState ) = 0;
	
	/**
	 * Set texture.
	 */
	virtual void setTexture( BcU32 Slot, class RsTexture* pTexture, BcBool Force = BcFalse ) = 0;

	/**
	 * Set program.
	 */
	virtual void setProgram( class RsProgram* Program ) = 0;

	/**
	 * Set index buffer.
	 */
	virtual void setIndexBuffer( class RsBuffer* IndexBuffer ) = 0;

	/**
	 * Set vertex buffer.
	 */
	virtual void setVertexBuffer( 
		BcU32 StreamIdx, 
		class RsBuffer* VertexBuffer,
		BcU32 Stride ) = 0;

	/**
	 * Set uniform buffer.
	 * @param SlotIdx Slot for uniform buffer to be bound.
	 * @param UniformBuffer Uniform buffer to be bound.
	 */
	virtual void setUniformBuffer( 
		BcU32 SlotIdx, 
		class RsBuffer* UniformBuffer ) = 0;

	/**
	 * Set vertex declaration.
	 */
	virtual void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) = 0;

	/**
	 * Set frame buffer.
	 */
	virtual void setFrameBuffer( class RsFrameBuffer* FrameBuffer ) = 0;

	/**
	 * Clear.
	 */
	virtual void clear( 
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil ) = 0;
	
	/**
	 * Draw primitives.
	 */
	virtual void drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices ) = 0;

	/**
	 * Draw indexed primitives.
	 */
	virtual void drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset ) = 0;
};

#endif

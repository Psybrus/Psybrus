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
	 * @post Frame buffer is set to specified one, or backbuffer if nullptr.
	 * @post Viewport will be set to size of frame buffer.
	 * @post Scissor rect will be set to size of frame buffer.
	 */
	virtual void setFrameBuffer( class RsFrameBuffer* FrameBuffer ) = 0;

	/**
	 * Set viewport.
	 * @pre Viewport is no larger than the currently bound frame buffer.
	 * @post Viewport will be set to specified one.
	 * @post Scissor rect will be set to size of viewport.
	 */
	virtual void setViewport( const class RsViewport& Viewport ) = 0;

	/**
	 * Set scissor rect.
	 */
	virtual void setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height ) { BcBreakpoint; };

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
	PSY_DEPRECATED( "Please use the stateless version of this function." )
	virtual void drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices ) = 0;

	/**
	 * Draw indexed primitives.
	 */
	PSY_DEPRECATED( "Please use the stateless version of this function." )
	virtual void drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset ) = 0;

	/**
	 * Draw primitives.
	 */
	virtual void drawPrimitives( 
		class RsGeometryBinding* GeometryBinding, 
		class RsProgramBinding* ProgramBinding, 
		class RsRenderState* RenderState,
		class RsFrameBuffer* FrameBuffer,
		RsTopologyType PrimitiveType, 
		BcU32 IndexOffset, BcU32 NoofIndices ) { BcBreakpoint; };

	/**
	 * Draw indexed primitives.
	 */
	virtual void drawIndexedPrimitives( 
		class RsGeometryBinding* GeometryBinding, 
		class RsProgramBinding* ProgramBinding, 
		class RsRenderState* RenderState,
		class RsFrameBuffer* FrameBuffer,
		RsTopologyType PrimitiveType, 
		BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset ) { BcBreakpoint; };

	/**
	 * Copy framebuffer render target to texture.
	 */
	virtual void copyFrameBufferRenderTargetToTexture( RsFrameBuffer* FrameBuffer, BcU32 Idx, RsTexture* Texture ) { BcBreakpoint; };

	/**
	 * Copy texture to framebuffer render target.
	 */
	virtual void copyTextureToFrameBufferRenderTarget( RsTexture* Texture, RsFrameBuffer* FrameBuffer, BcU32 Idx ) { BcBreakpoint; };
};

#endif

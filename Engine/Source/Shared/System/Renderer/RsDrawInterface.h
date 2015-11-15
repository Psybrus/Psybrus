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
	 * Clear.
	 * @param FrameBuffer Frame buffer we wish to render to. nullptr for backbuffer.
	 * @param Colour Colour to clear to.
	 * @param EnableClearColour Should we clear colour target?
	 * @param EnableClearDepth Should we clear depth target?
	 * @param EnableClearStencil Should we clear stencil target?
	 */
	virtual void clear( 
		const RsFrameBuffer* FrameBuffer,
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil ) = 0;
	
	/**
	 * Draw primitives.
	 * @param GeometryBinding Geometry to bind for draw.
	 * @param ProgramBinding Program + resource to bind for draw.
	 * @param RenderState Render state to draw with.
	 * @param FrameBufer Frame buffer we wish to render to. nullptr for backbuffer.
	 * @param Viewport Viewport to render to. nullptr for full @a FrameBuffer.
	 * @param ScissorRect Scissor rect to set. nullptr for full @a Viewport.
	 * @param PrimitiveType Type of primitive to draw.
	 * @param VertexOffset How many vertices to start rendering in from.
	 * @param NoofVertices How many vertices to draw.
	 */
	virtual void drawPrimitives( 
		const class RsGeometryBinding* GeometryBinding, 
		const class RsProgramBinding* ProgramBinding, 
		const class RsRenderState* RenderState,
		const class RsFrameBuffer* FrameBuffer,
		const struct RsViewport* Viewport,
		const struct RsScissorRect* ScissorRect,
		RsTopologyType PrimitiveType, 
		BcU32 VertexOffset, BcU32 NoofVertices ) { BcBreakpoint; };

	/**
	 * Draw indexed primitives.
	 * @param GeometryBinding Geometry to bind for draw.
	 * @param ProgramBinding Program + resource to bind for draw.
	 * @param RenderState Render state to draw with.
	 * @param FrameBufer Frame buffer we wish to render to. nullptr for backbuffer.
	 * @param Viewport Viewport to render to. nullptr for full @a FrameBuffer.
	 * @param ScissorRect Scissor rect to set. nullptr for full @a Viewport.
	 * @param PrimitiveType Type of primitive to draw.
	 * @param IndexOffset How many indices to start rendering in from.
	 * @param NoofIndices How many indices to draw.
	 * @param VertexOffset Base vertex offset to index from.
	 */
	virtual void drawIndexedPrimitives( 
		const class RsGeometryBinding* GeometryBinding, 
		const class RsProgramBinding* ProgramBinding, 
		const class RsRenderState* RenderState,
		const class RsFrameBuffer* FrameBuffer,
		const struct RsViewport* Viewport,
		const struct RsScissorRect* ScissorRect,
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

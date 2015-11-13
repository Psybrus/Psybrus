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
	 */
	virtual void clear( 
		RsFrameBuffer* FrameBuffer,
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil ) = 0;
	
	/**
	 * Draw primitives.
	 */
	virtual void drawPrimitives( 
		class RsGeometryBinding* GeometryBinding, 
		class RsProgramBinding* ProgramBinding, 
		class RsRenderState* RenderState,
		class RsFrameBuffer* FrameBuffer,
		const struct RsViewport* Viewport,
		const struct RsScissorRect* ScissorRect,
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

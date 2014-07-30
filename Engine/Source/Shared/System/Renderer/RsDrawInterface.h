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
	virtual void setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force = BcFalse ) = 0;
	
	/**
	 * Get render state.
	 */
	virtual BcS32 getRenderState( RsRenderStateType State ) const = 0;

	/**
	 * Set texture state.
	 */
	virtual void setTextureState( BcU32 Sampler, class RsTexture* pTexture, const RsTextureParams& Params, BcBool Force = BcFalse ) = 0;

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
		class RsVertexBuffer* VertexBuffer,
		BcU32 Stride ) = 0;

	/**
	 * Set vertex declaration.
	 */
	virtual void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) = 0;

	/**
	 * Clear.
	 */
	virtual void clear( const RsColour& Colour ) = 0;
	
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

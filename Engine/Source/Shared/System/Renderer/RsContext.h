/**************************************************************************
*
* File:		RsContext.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXT_H__
#define __RSCONTEXT_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class OsClient;

//////////////////////////////////////////////////////////////////////////
// RsContext
class RsContext:
	public RsResource
{
public:
	enum
	{
		MAX_VERTEX_STREAMS = 16
	};

public:
	RsContext( RsContext* pParentContext ):
		RsResource( pParentContext )
	{}
	virtual ~RsContext();

	/**
 	 * Get width.
	 */
	virtual BcU32 getWidth() const = 0;

	/**
 	 * Get height.
	 */
	virtual BcU32 getHeight() const = 0;

	/** 
	 * Is shader code type supported?
	 */
	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const = 0;

	/** 
	 * Get max shader type supported.
	 */
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const = 0;

	/**
	 * Requests a screenshot from the context.
	 */
	virtual void takeScreenshot() = 0;

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
	virtual void setIndexBuffer( class RsIndexBuffer* IndexBuffer ) = 0;

	/**
	 * Set vertex buffer.
	 */
	virtual void setVertexBuffer( BcU32 StreamIdx, class RsVertexBuffer* VertexBuffer ) = 0;

	/**
	 * Set vertex declaration.
	 */
	virtual void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) = 0;

	/**
	 * Flush state.
	 */
	virtual void flushState() = 0;

	/**
	 * Clear.
	 */
	virtual void clear( const RsColour& Colour ) = 0;
	
	/**
	 * Draw primitives.
	 */
	virtual void drawPrimitives( RsPrimitiveType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices ) = 0;

	/**
	 * Draw indexed primitives.
	 */
	virtual void drawIndexedPrimitives( RsPrimitiveType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset ) = 0;
};


#endif

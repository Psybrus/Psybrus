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
#include "System/Renderer/RsDrawInterface.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class OsClient;

//////////////////////////////////////////////////////////////////////////
// RsContext
class RsContext:
	public RsResource,
	public RsDrawInterface
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
	 * Set render target.
	 */
	virtual void setRenderTarget( class RsRenderTarget* RenderTarget ) = 0;

	/**
	 * Set viewport.
	 */
	virtual void setViewport( class RsViewport& Viewport ) = 0;
};

#endif

/**************************************************************************
*
* File:		RsRenderTarget.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Render target.
*		
*
*
* 
**************************************************************************/

#ifndef __RSRENDERTARGET_H__
#define __RSRENDERTARGET_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
/**	\class RsRenderTarget
*	\brief Render target.
*
*	Abstract render target resource. 
*/
class RsRenderTarget:
	public RsResource
{
public:
	RsRenderTarget( class RsContext* pContext ):
		RsResource( pContext )
	{}

	virtual ~RsRenderTarget()
	{}

	/**
	 *	Get width.
	 */
	virtual BcU32						width() const = 0;

	/**
	 *	Get height.
	 */
	virtual BcU32						height() const = 0;

	/**
	 *	Get colour format.
	 */
	virtual eRsColourFormat				colourFormat( BcU32 Index ) const = 0;

	/**
	 *	Get depth stencil format.
	 */
	virtual eRsDepthStencilFormat		depthStencilFormat() const = 0;
	
	/**
	 * Get texture that we render to.
	 */
	virtual RsTexture*					getTexture() = 0;
};

#endif

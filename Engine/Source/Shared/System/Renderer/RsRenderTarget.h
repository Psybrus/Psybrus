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

/**
* Render target descriptor.
*/
struct RsRenderTargetDesc
{
	RsRenderTargetDesc( eRsColourFormat ColourFormat, 
						eRsDepthStencilFormat DepthStencilFormat,
						BcU32 Width,
						BcU32 Height );

	RsRenderTargetDesc( BcU32 NoofColourFormats,
						eRsColourFormat* ColourFormats, 
						eRsDepthStencilFormat DepthStencilFormat,
						BcU32 Width,
						BcU32 Height );

	std::array< eRsColourFormat, 8 >	ColourFormats_;
	eRsDepthStencilFormat				DepthStencilFormat_;
	BcU32								Width_;
	BcU32								Height_;

};

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
	enum
	{
		MAX_COLOUR_TARGETS = 8
	};

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
	virtual RsTexture*					getTexture( BcU32 Index ) = 0;
};

#endif

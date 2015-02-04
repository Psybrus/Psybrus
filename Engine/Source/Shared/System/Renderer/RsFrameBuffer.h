/**************************************************************************
*
* File:		RsFrameBuffer.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Frame buffer handling.
*		
*
*
* 
**************************************************************************/

#ifndef __RSFRAMEBUFFER_H__
#define __RSFRAMEBUFFER_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsFrameBufferDesc
struct RsFrameBufferDesc
{
	RsFrameBufferDesc( BcU32 NoofTargets );

	/**
	 * Set render target.
	 */
	RsFrameBufferDesc& setRenderTarget( BcU32 Idx, RsTexture* Texture );

	/**
	 * Set depth stencil target.
	 */
	RsFrameBufferDesc& setDepthStencilTarget( RsTexture* Texture );

	std::vector< class RsTexture* > RenderTargets_;
	RsTexture* DepthStencilTarget_;
};

//////////////////////////////////////////////////////////////////////////
// RsFrameBuffer
class RsFrameBuffer:
	public RsResource
{
public:
	RsFrameBuffer( RsContext* pContext, const RsFrameBufferDesc& Desc );
	virtual ~RsFrameBuffer(){};

	/**
	 * Get descriptor.
	 */
	const RsFrameBufferDesc& getDesc() const;
	
	
protected:
	RsFrameBufferDesc Desc_;
};

#endif

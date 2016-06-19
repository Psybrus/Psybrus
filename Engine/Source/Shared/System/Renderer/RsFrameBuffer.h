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
	RsFrameBufferDesc( size_t NoofTargets );

	/**
	 * Set render target.
	 */
	RsFrameBufferDesc& setRenderTarget( size_t Idx, class RsTexture* Texture, BcU32 Level = 0, RsTextureFace Face = RsTextureFace::NONE );

	/**
	 * Set depth stencil target.
	 */
	RsFrameBufferDesc& setDepthStencilTarget( RsTexture* Texture );

	struct RTV
	{
		class RsTexture* Texture_ = nullptr;
		BcU32 Level_ = 0;
		RsTextureFace Face_ = RsTextureFace::NONE;
	};
	
	// TODO: Get rid of vector.
	std::vector< RTV > RenderTargets_;
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

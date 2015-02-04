/**************************************************************************
*
* File:		RsFrameBuffer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// RsFrameBufferDesc
RsFrameBufferDesc::RsFrameBufferDesc( BcU32 NoofTargets ):
	RenderTargets_( NoofTargets, nullptr ),
	DepthStencilTarget_( nullptr )
{
}


//////////////////////////////////////////////////////////////////////////
// setRenderTarget
RsFrameBufferDesc& RsFrameBufferDesc::setRenderTarget( BcU32 Idx, RsTexture* Texture )
{
	BcAssertMsg( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE,
		"Can't bind texture as render target, was it created with RsResourceBindFlags::RENDER_TARGET?" );
	RenderTargets_[ Idx ] = Texture;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// setDepthStencilTarget
RsFrameBufferDesc& RsFrameBufferDesc::setDepthStencilTarget( RsTexture* Texture )
{
	BcAssertMsg( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE,
		"Can't bind texture as depth stencil target, was it created with RsResourceBindFlags::DEPTH_STENCIL?" );
	DepthStencilTarget_ = Texture;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBuffer::RsFrameBuffer( RsContext* pContext, const RsFrameBufferDesc& Desc ):
	RsResource( pContext ),
	Desc_( Desc )
{

}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsFrameBufferDesc& RsFrameBuffer::getDesc() const
{
	return Desc_;
}

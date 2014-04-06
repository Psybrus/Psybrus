/**************************************************************************
*
* File:		RsRenderTargetGL.h
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RsRenderTargetGL_H__
#define __RsRenderTargetGL_H__

#include "Base/BcTypes.h"
#include "System/Renderer/RsRenderTarget.h"

#include "System/Renderer/GL/RsRenderBufferGL.h"
#include "System/Renderer/GL/RsFrameBufferGL.h"
#include "System/Renderer/GL/RsTextureGL.h"

//////////////////////////////////////////////////////////////////////////
// RsRenderTargetGL
class RsRenderTargetGL:
	public RsRenderTarget
{
public:
	RsRenderTargetGL( RsContext* pContext, const RsRenderTargetDesc& Desc, RsRenderBufferGL* pColourBuffer, RsRenderBufferGL* pDepthStencilBuffer, RsFrameBufferGL* pFrameBuffer, RsTextureGL* pTexture );
	virtual ~RsRenderTargetGL();
	
	void								bind();
	
protected:
	virtual BcU32						width() const;
	virtual BcU32						height() const;
	virtual eRsColourFormat				colourFormat( BcU32 Index ) const;
	virtual eRsDepthStencilFormat		depthStencilFormat() const;
	virtual RsTexture*					getTexture( BcU32 Index );

protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

private:
	RsRenderTargetDesc					Desc_;
	RsRenderBufferGL*					pColourBuffer_;
	RsRenderBufferGL*					pDepthStencilBuffer_;
	RsFrameBufferGL*					pFrameBuffer_;
	RsTextureGL*						pTexture_;
};

#endif



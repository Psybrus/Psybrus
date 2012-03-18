/**************************************************************************
*
* File:		RsRenderBufferGL.h
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RsRenderBufferGL_H__
#define __RsRenderBufferGL_H__

#include "Base/BcTypes.h"
#include "System/Renderer/RsResource.h"
#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsRenderBufferGL
class RsRenderBufferGL:
	public RsResource
{
public:
	RsRenderBufferGL( eRsColourFormat Format, BcU32 Width, BcU32 Height );
	RsRenderBufferGL( eRsDepthStencilFormat Format, BcU32 Width, BcU32 Height );
	virtual ~RsRenderBufferGL();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

private:
	GLint								Format_;
	BcU32								Width_;
	BcU32								Height_;

	
};

#endif



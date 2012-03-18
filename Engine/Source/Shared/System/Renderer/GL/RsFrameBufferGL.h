/**************************************************************************
*
* File:		RsFrameBufferGL.h
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RsFrameBufferGL_H__
#define __RsFrameBufferGL_H__

#include "Base/BcTypes.h"
#include "System/Renderer/RsResource.h"
#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// RsFrameBufferGL
class RsFrameBufferGL:
	public RsResource
{
public:
	RsFrameBufferGL();
	virtual ~RsFrameBufferGL();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
private:
	
	
};


#endif



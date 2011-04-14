/**************************************************************************
*
* File:		RsBufferGLES.h
* Author:	Neil Richardson 
* Ver/Date:	25/02/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSBUFFERGLES_H__
#define __RSBUFFERGLES_H__

#include "RsGPUResourceGLES.h"

////////////////////////////////////////////////////////////////////////////////
// RsBufferGLES
class RsBufferGLES: public RsGPUResourceGLES
{
public:
	RsBufferGLES();
	virtual ~RsBufferGLES();
	
protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
protected:
	GLenum								Type_;
	GLenum								Usage_;
};

#endif

/**************************************************************************
*
* File:		RsGPUResourceGLES.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GLES GPU resource.
*		
*
*
* 
**************************************************************************/

#ifndef __RSGPURESOURCEGLES_H__
#define __RSGPURESOURCEGLES_H__

#include "RsGLES.h"
#include "RsTypes.h"

////////////////////////////////////////////////////////////////////////////////
// RsGPUResourceGLES
class RsGPUResourceGLES
{
public:
	RsGPUResourceGLES();
	virtual ~RsGPUResourceGLES();
	
	virtual void						create() = 0;
	virtual void						update() = 0;
	virtual void						destroy() = 0;	
	
public:
	GLuint								getHandle() const;
	
protected:
	GLuint								Handle_;
	void*								pData_;
	BcU32								DataSize_;
	BcBool								DeleteData_;	
};

////////////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline GLuint RsGPUResourceGLES::getHandle() const
{
	return Handle_;
}

#endif

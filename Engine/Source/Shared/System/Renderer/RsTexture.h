/**************************************************************************
*
* File:		RsTexture.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Simple Texture handling class
*		
*
*
* 
**************************************************************************/

#ifndef __RSTEXTURE_H__
#define __RSTEXTURE_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsTexture
class RsTexture:
	public RsResource
{
public:
	RsTexture(){};
	virtual ~RsTexture(){};

	virtual BcU32 width() const = 0;
	virtual BcU32 height() const = 0;
	virtual BcU32 depth() const = 0;
	virtual BcU32 levels() const = 0;
	virtual eRsTextureType type() const = 0;
	virtual eRsTextureFormat format() const = 0;

	// Editing.
	virtual void* lockTexture() = 0;
	virtual void unlockTexture() = 0;

private:
	
};

#endif

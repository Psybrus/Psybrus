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
// RsTextureDesc
struct RsTextureDesc
{
	RsTextureDesc();

	RsTextureType Type_;
	RsTextureFormat Format_;
	BcU32 Width_;
	BcU32 Height_;
	BcU32 Depth_;	
};

//////////////////////////////////////////////////////////////////////////
// RsTexture
class RsTexture:
	public RsResource
{
public:
	RsTexture( RsContext* pContext ):
		RsResource( pContext )
	{};
	virtual ~RsTexture(){};

	virtual BcU32 width() const = 0;
	virtual BcU32 height() const = 0;
	virtual BcU32 depth() const = 0;
	virtual BcU32 levels() const = 0;
	virtual RsTextureType type() const = 0;
	virtual RsTextureFormat format() const = 0;

	// Editing.
	virtual void* lockTexture() = 0;
	virtual void unlockTexture() = 0;

private:
	
};

#endif

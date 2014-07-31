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
	RsTextureDesc( 
		RsTextureType Type, 
		RsTextureFormat Format,
		BcU32 Levels,
		BcU32 Width, 
		BcU32 Height = 0,
		BcU32 Depth = 0 );

	RsTextureType Type_;
	RsTextureFormat Format_;
	BcU32 Levels_;
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


	// Old.
	virtual BcU32 width() const = 0;
	virtual BcU32 height() const = 0;
	virtual BcU32 depth() const = 0;
	virtual BcU32 levels() const = 0;
	virtual RsTextureType type() const = 0;
	virtual RsTextureFormat format() const = 0;

	// Editing.
	virtual void* lockTexture() = 0;
	virtual void unlockTexture() = 0;

protected:
	RsTextureDesc Desc_;
};

#endif

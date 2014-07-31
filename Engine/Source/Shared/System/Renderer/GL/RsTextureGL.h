/**************************************************************************
*
* File:		RsTextureGL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RsTextureGL_H__
#define __RsTextureGL_H__

#include "System/Renderer/RsTexture.h"
#include "System/Renderer/GL/RsGL.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsCoreImplGL;

//////////////////////////////////////////////////////////////////////////
// RsTextureGL
class RsTextureGL:
	public RsTexture
{
public:
	RsTextureGL( RsContext* pContext, const RsTextureDesc& Desc, void* pTextureData );
	virtual ~RsTextureGL();
	
	// Editing.
	virtual void*						lockTexture();
	virtual void						unlockTexture();
	
	// Utility.
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
protected:
	void								loadTexture1D();
	void								loadTexture2D();
	void								loadTexture3D();
	void								loadTextureCubeMap();

private:
	BcBool								Locked_;		// BADNESS: Needs to be atomic!

};


#endif

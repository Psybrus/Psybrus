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
	// 1D
	RsTextureGL( RsContext* pContext, BcU32 Width, BcU32 Levels, eRsTextureFormat Format, void* pTextureData );

	// 2D
	RsTextureGL( RsContext* pContext, BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pTextureData );

	// 3D
	RsTextureGL( RsContext* pContext, BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, eRsTextureFormat Format, void* pTextureData );
	virtual ~RsTextureGL();
	
	virtual BcU32						width() const;
	virtual BcU32						height() const;
	virtual BcU32						depth() const;
	virtual BcU32						levels() const;
	virtual eRsTextureType				type() const;
	virtual eRsTextureFormat			format() const;

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
	BcU32								Width_;
	BcU32								Height_;
	BcU32								Depth_;
	BcU32								Levels_;
	eRsTextureType						Type_;
	eRsTextureFormat					Format_;
	BcBool								Locked_;		// BADNESS: Needs to be atomic!

};


#endif

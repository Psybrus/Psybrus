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

#include "RsTexture.h"
#include "RsGL.h"

#include "CsFile.h"
#include "BcAtomicMutex.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsCoreImplGL;

//////////////////////////////////////////////////////////////////////////
// RsTextureGL
class RsTextureGL:
	public RsTexture
{
public:
	/**
	 * Create texture.
	 * @param Width Width.
	 * @param Height Height.
	 * @param Levels Mipmap Levels.
	 * @param Format Texture format.
	 * @param pTextureData Texture data.
	 */
	RsTextureGL( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pTextureData );
	virtual ~RsTextureGL();
	
	virtual BcU32						width() const;
	virtual BcU32						height() const;
	
	// Editing.
	virtual void*						lockTexture();
	virtual void						unlockTexture();
	
	// Utility.
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	
	
private:
	BcU32								Width_;
	BcU32								Height_;
	BcU32								Levels_;
	eRsTextureFormat					Format_;
	BcBool								Locked_;		// BADNESS: Needs to be atomic!

};


#endif

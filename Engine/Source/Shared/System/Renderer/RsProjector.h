/**************************************************************************
*
* File:		RsProejctor.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Texture projector class.
*		
*
*
* 
**************************************************************************/

#ifndef __RSPROJECTOR_H__
#define __RSPROJECTOR_H__

#include "System/Renderer/RsTypes.h"

#include "System/Renderer/RsViewport.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class RsTexture;

//////////////////////////////////////////////////////////////////////////
// RsProjector
class RsProjector:
	public RsViewport
{
public:
	RsProjector();
	~RsProjector();

	/**
	*	Set texture to use with projection.
	*/
	void					texture( RsTexture* pTexture );
	RsTexture*				texture();

private:
	RsTexture*				pTexture_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline RsProjector::RsProjector()
{
	pTexture_ = NULL;
}

inline RsProjector::~RsProjector()
{

}

inline void RsProjector::texture( RsTexture* pTexture )
{
	pTexture_ = pTexture;
}

inline RsTexture* RsProjector::texture()
{
	return pTexture_;
}


#endif

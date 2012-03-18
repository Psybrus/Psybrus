/**************************************************************************
*
* File:		ScnRenderTarget.h
* Author:	Neil Richardson 
* Ver/Date:	16/08/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnRenderTarget_H__
#define __ScnRenderTarget_H__

#include "Base/BcTypes.h"

#include "System/Scene/ScnTexture.h"
#include "System/Scene/ScnTypes.h"

#include "System/Renderer/RsRenderTarget.h"

//////////////////////////////////////////////////////////////////////////
// ScnRenderTargetRef
typedef CsResourceRef< class ScnRenderTarget > ScnRenderTargetRef;

//////////////////////////////////////////////////////////////////////////
// ScnRenderTarget
class ScnRenderTarget:
	public ScnTexture
{
public:
	DECLARE_RESOURCE( ScnTexture, ScnRenderTarget );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	virtual void						initialise( BcU32 Width, BcU32 Height );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	void								bind( RsFrame* pFrame );
	void								unbind( RsFrame* pFrame );
	
protected:
	RsRenderTarget*						pRenderTarget_;
	
	THeader								TextureHeader_;
};


#endif



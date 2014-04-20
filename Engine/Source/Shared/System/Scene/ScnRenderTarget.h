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
typedef ReObjectRef< class ScnRenderTarget > ScnRenderTargetRef;

//////////////////////////////////////////////////////////////////////////
// ScnRenderTarget
class ScnRenderTarget:
	public ScnTexture
{
public:
	DECLARE_RESOURCE( ScnTexture, ScnRenderTarget );
	
#if PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	virtual void						initialise( BcU32 Width, BcU32 Height );
	virtual void						create();
	virtual void						destroy();
	
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

	void								bind( RsFrame* pFrame );
	void								unbind( RsFrame* pFrame );
	
protected:
	RsRenderTarget*						pRenderTarget_;

};


#endif



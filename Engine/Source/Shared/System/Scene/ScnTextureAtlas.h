/**************************************************************************
*
* File:		ScnTextureAtlas.h
* Author:	Neil Richardson 
* Ver/Date:	24/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnTextureAtlas_H__
#define __ScnTextureAtlas_H__

#include "BcTypes.h"

#include "ScnTexture.h"
#include "ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef CsResourceRef< class ScnTextureAtlas > ScnTextureAtlasRef;

//////////////////////////////////////////////////////////////////////////
// ScnTextureAtlas
class ScnTextureAtlas:
	public ScnTexture
{
public:
	DECLARE_RESOURCE( ScnTexture, ScnTextureAtlas );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	

	virtual const ScnRect&				getRect( BcU32 Idx );
	virtual BcU32						noofRects();

protected:
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );

protected:
	struct TAtlasHeader
	{
		BcU32 NoofTextures_;
	};
	
	TAtlasHeader* pAtlasHeader_;	
	ScnRect* pAtlasRects_;
};



#endif



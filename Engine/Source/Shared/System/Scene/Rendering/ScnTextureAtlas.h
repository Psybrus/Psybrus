/**************************************************************************
*
* File:		Rendering/ScnTextureAtlas.h
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

#include "Base/BcTypes.h"

#include "System/Scene/Rendering/ScnTexture.h"

#include "System/Scene/Rendering/ScnTextureAtlasFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef ReObjectRef< class ScnTextureAtlas > ScnTextureAtlasRef;

//////////////////////////////////////////////////////////////////////////
// ScnTextureAtlas
class ScnTextureAtlas:
	public ScnTexture
{
public:
	REFLECTION_DECLARE_DERIVED( ScnTextureAtlas, ScnTexture );

	ScnTextureAtlas();
	virtual ~ScnTextureAtlas();

	const ScnRect& getRect( BcU32 Idx ) const override;
	BcU32 noofRects() const override;

protected:
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;

protected:
	struct ScnTextureAtlasHeader
	{
		BcU32 NoofTextures_;
	};
	
	ScnTextureAtlasHeader* pAtlasHeader_;	
	ScnTextureAtlasRect* pAtlasRects_;
};



#endif



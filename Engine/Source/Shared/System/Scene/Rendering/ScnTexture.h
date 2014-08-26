/**************************************************************************
*
* File:		Rendering/ScnTexture.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNTEXTURE_H__
#define __SCNTEXTURE_H__

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/Rendering/ScnTextureFileData.h"


//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef ReObjectRef< class ScnTexture > ScnTextureRef;
typedef std::vector< ScnTextureRef > ScnTextureList;
typedef ScnTextureList::iterator ScnTextureListIterator;
typedef ScnTextureList::const_iterator ScnTextureListConstIterator;
typedef std::map< BcName, ScnTextureRef > ScnTextureMap;
typedef ScnTextureMap::iterator ScnTextureMapIterator;
typedef ScnTextureMap::const_iterator ScnTextureMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnTexture
class ScnTexture:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnTexture, CsResource );
	
	virtual void						initialise();
	virtual void						initialise( BcU32 Width, BcU32 Levels, RsTextureFormat Format );
	virtual void						initialise( BcU32 Width, BcU32 Height, BcU32 Levels, RsTextureFormat Format );
	virtual void						initialise( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, RsTextureFormat Format );
	virtual void						create();
	virtual void						destroy();
	
	RsTexture*							getTexture();
	
	BcU32								getWidth() const;
	BcU32								getHeight() const;
	
	virtual const ScnRect&				getRect( BcU32 Idx );
	virtual BcU32						noofRects();
	
protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

protected:
	RsTexture*							pTexture_;
	
	ScnTextureHeader					Header_;
	void*								pTextureData_;
};

#endif

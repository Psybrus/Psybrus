/**************************************************************************
*
* File:		ScnTexture.h
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

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef CsResourceRef< class ScnTexture > ScnTextureRef;
typedef std::list< ScnTextureRef > ScnTextureList;
typedef ScnTextureList::iterator ScnTextureListIterator;
typedef ScnTextureList::const_iterator ScnTextureListConstIterator;
typedef std::map< std::string, ScnTextureRef > ScnTextureMap;
typedef ScnTextureMap::iterator ScnTextureMapIterator;
typedef ScnTextureMap::const_iterator ScnTextureMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnTexture
class ScnTexture:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnTexture );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	RsTexture*							getTexture();
	
protected:
	virtual void						setup();
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );

protected:
	RsTexture*							pTexture_;
	
	struct THeader
	{
		BcU32							Width_;
		BcU32							Height_;
		BcU32							Levels_;
		eRsTextureFormat				Format_;
	};
	
	THeader*							pHeader_;
	void*								pTextureData_;
	BcBool								CreateNewTexture_;
};

#endif

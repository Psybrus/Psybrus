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

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureRef
typedef CsResourceRef< class ScnTexture > ScnTextureRef;
typedef std::vector< ScnTextureRef > ScnTextureList;
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
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	virtual void						initialise();
	virtual void						initialise( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	RsTexture*							getTexture();
	
	BcU32								getWidth() const;
	BcU32								getHeight() const;
	RsColour							getTexel( BcU32 X, BcU32 Y ) const;
	void								setTexel( BcU32 X, BcU32 Y, const RsColour& Colour );
	void								lock();
	void								unlock();
	
	virtual const ScnRect&				getRect( BcU32 Idx );
	virtual BcU32						noofRects();
	
protected:
	virtual void						setup();
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

protected:
	RsTexture*							pTexture_;
	
	struct THeader
	{
		BcU32							Width_;
		BcU32							Height_;
		BcU32							Levels_;
		eRsTextureFormat				Format_;
	};
	
	THeader								Header_;
	void*								pTextureData_;
	BcBool								CreateNewTexture_;
	BcBool								IsUserCreated_;
};

#endif

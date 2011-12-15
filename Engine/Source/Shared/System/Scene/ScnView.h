/**************************************************************************
*
* File:		ScnView.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNVIEW_H__
#define __SCNVIEW_H__

#include "RsCore.h"
#include "CsResource.h"

#include "ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewRef
typedef CsResourceRef< class ScnView > ScnViewRef;
typedef std::vector< ScnViewRef > ScnViewList;
typedef ScnViewList::iterator ScnViewListIterator;
typedef ScnViewList::const_iterator ScnViewListConstIterator;
typedef std::map< std::string, ScnViewRef > ScnViewMap;
typedef ScnViewMap::iterator ScnViewMapIterator;
typedef ScnViewMap::const_iterator ScnViewMapConstIterator;

//////////////////////////////////////////////////////////////////////////
// ScnView
class ScnView:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnView );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:
	RsViewport							Viewport_;
	
	struct THeader
	{
		// Viewport.
		BcU32							X_;
		BcU32							Y_;
		BcU32							Width_;
		BcU32							Height_;

		// Perspective projection.
		BcReal							FieldOfView_;
		BcReal							Near_;
		BcReal							Far_;
	};
	
	THeader*							pHeader_;
};

#endif

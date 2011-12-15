/**************************************************************************
*
* File:		ScnEntity.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnEntity_H__
#define __ScnEntity_H__

#include "RsCore.h"
#include "CsResource.h"

#include "ScnTypes.h"

#include "ScnComponent.h"

#include "ScnTransform.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntity
class ScnEntity:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnEntity );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
public:
	virtual void						update( BcReal Tick );
	virtual void						attach( ScnComponentRef Component );
	virtual void						detach( ScnComponentRef Component );

protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:

	struct THeader
	{
	};
	
	THeader*							pHeader_;

	ScnComponentList					Components_;
	ScnTransform						Transform_;
};

#endif

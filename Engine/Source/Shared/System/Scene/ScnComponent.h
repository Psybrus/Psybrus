/**************************************************************************
*
* File:		ScnComponent.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnComponent_H__
#define __ScnComponent_H__

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnComponent
class ScnComponent:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnComponent );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:

	struct THeader
	{
	};
	
	THeader*							pHeader_;

	ScnEntityWeakRef					Parent_;
};

#endif

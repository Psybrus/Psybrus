/**************************************************************************
*
* File:		ScnScript.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNSCRIPT_H__
#define __SCNSCRIPT_H__

#include "RsCore.h"
#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// ScnScriptRef
typedef CsResourceRef< class ScnScript > ScnScriptRef;

//////////////////////////////////////////////////////////////////////////
// ScnScript
class ScnScript:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnScript );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	int									execute();
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
private:
	const char*							pScript_;
	
};

#endif

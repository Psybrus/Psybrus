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
	DECLARE_RESOURCE( ScnScript );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
		
private:
	void								fileReady();
	void								fileChunkReady( const CsFileChunk* pChunk, void* pData );
	
private:
	
};

#endif

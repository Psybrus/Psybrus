/**************************************************************************
*
* File:		ScnPackage.h
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNPACKAGE_H__
#define __SCNPACKAGE_H__

#include "RsCore.h"
#include "CsResourceRef.h"

//////////////////////////////////////////////////////////////////////////
// ScnScriptRef
typedef CsResourceRef< class ScnPackage > ScnPackageRef;

//////////////////////////////////////////////////////////////////////////
// ScnPackage
class ScnPackage:
	public CsResource
{
public:
	DECLARE_RESOURCE( ScnPackage );
	
#ifdef PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
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

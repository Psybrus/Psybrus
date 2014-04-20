/**************************************************************************
*
* File:		ScnSound.h
* Author:	Neil Richardson 
* Ver/Date:	28/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSound_H__
#define __ScnSound_H__

#include "System/Content/CsResource.h"

#include "System/Sound/SsSample.h"

#include "System/Scene/ScnSoundFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundRef
typedef ReObjectRef< class ScnSound > ScnSoundRef;

//////////////////////////////////////////////////////////////////////////
// ScnSound
class ScnSound:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnSound );
	
#ifdef PSY_SERVER
	virtual BcBool						import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	
	SsSample*							getSample();
	
protected:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	SsSample*							pSample_;
	
	ScnSoundHeader*						pHeader_;
	void*								pSampleData_;
	BcU32								SampleDataSize_;

	// Ogg stuff.
};


#endif



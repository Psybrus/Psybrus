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

//////////////////////////////////////////////////////////////////////////
// ScnSoundRef
typedef CsResourceRef< class ScnSound > ScnSoundRef;

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
	virtual BcBool						isReady();
	
	SsSample*							getSample();
	
	void								setup();
	
protected:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	SsSample*							pSample_;

	struct THeader
	{
		BcU32							SampleRate_;
		BcU32							Channels_;
		BcBool							Looping_;
		BcBool							IsOgg_;
	};
	
	THeader*							pHeader_;
	void*								pSampleData_;
	BcU32								SampleDataSize_;

	// Ogg stuff.
};


#endif



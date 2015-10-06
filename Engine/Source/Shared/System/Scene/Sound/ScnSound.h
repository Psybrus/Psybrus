/**************************************************************************
*
* File:		Sound/ScnSound.h
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

#include "System/Sound/SsSource.h"
#include "System/Scene/Sound/ScnSoundFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundRef
typedef ReObjectRef< class ScnSound > ScnSoundRef;

//////////////////////////////////////////////////////////////////////////
// ScnSound
class ScnSound:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnSound, CsResource );
	
	ScnSound();
	virtual ~ScnSound();
	
	void create() override;
	void destroy() override;
	
	class SsSource* getSource();

protected:
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;
	
protected:
	SsSourceFileData* pFileData_;
	class SsSource* pSource_;
};


#endif



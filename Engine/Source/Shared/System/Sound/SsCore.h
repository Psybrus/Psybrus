/**************************************************************************
 *
 * File:		SsCore.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSCOREIMPL_H__
#define __SSCOREIMPL_H__

#include "System/Sound/SsTypes.h"
#include "System/Sound/SsChannel.h"
#include "System/Sound/SsSample.h"

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations
class SsCore;

//////////////////////////////////////////////////////////////////////////
// SsCore
class SsCore:
	public SysSystem,
	public BcGlobal< SsCore >
{
public:
	static BcU32 JOB_QUEUE_ID;

public:
	REFLECTION_DECLARE_DERIVED( SsCore, SysSystem );

public:
	SsCore(){};
	virtual ~SsCore(){};
	
public:
	virtual SsSample* createSample( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize ) = 0;
	virtual void updateResource( SsResource* pResource ) = 0;
	virtual void destroyResource( SsResource* pResource ) = 0;
	
public:
	virtual SsChannel* play( SsSample* Sample, SsChannelCallback* Callback = NULL ) = 0;
	virtual SsChannel* queue( SsSample* Sample, SsChannelCallback* Callback = NULL ) = 0;
	virtual void unregister( SsChannelCallback* Callback = NULL ) = 0;
	virtual void setListener( const MaVec3d& Position, const MaVec3d& LookAt, const MaVec3d& Up ) = 0;

};

#endif


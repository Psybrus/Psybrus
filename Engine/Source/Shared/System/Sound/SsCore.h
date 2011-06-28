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

#include "SsChannel.h"
#include "SsSample.h"

#include "BcGlobal.h"
#include "SysSystem.h"

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
	SsCore(){};
	virtual ~SsCore(){};
	
public:
	virtual SsSample* createSample( BcU32 SampleRate, BcU32 Channels, BcBool Looping, void* pData, BcU32 DataSize ) = 0;
	virtual void updateResource( SsResource* pResource ) = 0;
	virtual void destroyResource( SsResource* pResource ) = 0;
	
public:
	virtual SsChannel* play( SsSample* Sample, SsChannelCallback* Callback = NULL ) = 0;
	virtual void setListener( const BcVec3d& Position, const BcVec3d& LookAt, const BcVec3d& Up ) = 0;
};

#endif


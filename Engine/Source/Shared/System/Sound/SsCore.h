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

#include "Base/BcGlobal.h"
#include "System/SysSystem.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations
class SsCore;

//////////////////////////////////////////////////////////////////////////
// Callbacks
typedef std::function< void( class SsChannel* ) > SsChannelCallback;

//////////////////////////////////////////////////////////////////////////
// SsCore
class SsCore:
	public SysSystem,
	public BcGlobal< SsCore >
{
public:
	static size_t JOB_QUEUE_ID;

public:
	REFLECTION_DECLARE_DERIVED( SsCore, SysSystem );

public:
	SsCore(){};
	virtual ~SsCore(){};
	
public:
	virtual class SsBus* createBus(
		const class SsBusParams& Params ) = 0;

	virtual class SsFilter* createFilter( 
		const class SsFilterParams& Params ) = 0;

	virtual class SsSource* createSource( 
		const class SsSourceParams& Params,
		const struct SsSourceFileData* FileData ) = 0;

	virtual void destroyResource( 
		class SsBus* Resource ) = 0;
	
	virtual void destroyResource( 
		class SsFilter* Resource ) = 0;

	virtual void destroyResource( 
		class SsSource* Resource ) = 0;

	virtual class SsChannel* playSource( 
		class SsSource* Source, 
		const class SsChannelParams& Params,
		SsChannelCallback DoneCallback ) = 0;

	virtual void stopChannel( 
		class SsChannel* Channel,
		BcBool ForceFlush = BcFalse ) = 0;

	virtual void updateChannel(
		class SsChannel* Channel,
		const class SsChannelParams& Params ) = 0;

protected:
	void setChannelParams( class SsChannel* Channel, const class SsChannelParams& Params );

};

#endif


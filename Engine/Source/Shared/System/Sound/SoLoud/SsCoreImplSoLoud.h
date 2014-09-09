/**************************************************************************
 *
 * File:		SsCoreImplSoLoud.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSCOREIMPLSOLOUD_H__
#define __SSCOREIMPLSOLOUD_H__

#include "System/Sound/SsCore.h"

#include "System/Sound/SsBus.h"
#include "System/Sound/SsFilter.h"
#include "System/Sound/SsChannel.h"
#include "System/Sound/SsSource.h"

//////////////////////////////////////////////////////////////////////////
// Forward declarations.
namespace SoLoud
{
	class Soloud;
	typedef void (*mutexCallFunction)(void *aMutexPtr);
	typedef void (*soloudCallFunction)(Soloud *aSoloud);
	typedef unsigned int result;
	typedef unsigned int handle;
	typedef double time;
}

//////////////////////////////////////////////////////////////////////////
// SsCoreImplSoLoud
class SsCoreImplSoLoud:
	public SsCore
{
public:
	REFLECTION_DECLARE_DERIVED( SsCoreImplSoLoud, SsCore );

public:
	SsCoreImplSoLoud();
	virtual ~SsCoreImplSoLoud();

private:
	void open();
	void update();
	void close();

public:
	SsBus* createBus( 
		const SsBusParams& Params );
	
	SsFilter* createFilter( 
		const SsFilterParams& Params );

	SsSource* createSource( 
		const SsSourceParams& Params );

	void destroyResource( 
		SsBus* Resource );
	
	void destroyResource( 
		SsFilter* Resource );

	void destroyResource( 
		SsSource* Resource );

	SsChannel* playSource( 
		SsSource* Source, 
		const SsChannelParams& Params,
		SsChannelCallback DoneCallback );

	void stopChannel( 
		SsChannel* Channel );

	void updateChannel(
		SsChannel* Channel,
		const SsChannelParams& Params );

private:
	SsChannel* allocChannel();
	void updateChannel( SsChannel* Channel, SoLoud::handle Handle );
	void freeChannel( SsChannel* Channel );
	
public:
	SoLoud::Soloud* SoLoudCore_;
	std::list< SsChannel* > FreeChannels_;
	std::list< SsChannel* > PendingChannels_;
	std::list< SsChannel* > UsedChannels_;
	std::map< SsChannel*, SsChannelCallback > ChannelDoneCallbacks_;
	std::recursive_mutex ChannelMutex_;
	
};

#endif

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
	void open() override;
	void update() override;
	void close() override;

public:
	SsBus* createBus( 
		const SsBusParams& Params ) override;
	
	SsFilter* createFilter( 
		const SsFilterParams& Params ) override;

	SsSource* createSource( 
		const SsSourceParams& Params,
		const SsSourceFileData* FileData ) override;

	void destroyResource( 
		SsBus* Resource ) override;
	
	void destroyResource( 
		SsFilter* Resource ) override;

	void destroyResource( 
		SsSource* Resource ) override;

	SsChannel* playSource( 
		SsSource* Source, 
		const SsChannelParams& Params,
		SsChannelCallback DoneCallback ) override;

	void stopChannel( 
		SsChannel* Channel,
		BcBool ForceFlush ) override;

	void updateChannel(
		SsChannel* Channel,
		const SsChannelParams& Params ) override;

	void setListener( 
		const MaMat4d& Transform, const MaVec3d& Velocity ) override;

	void getVisualisationData( 
		std::vector< BcF32 >& OutFFT, std::vector< BcF32 >& OutWave ) override;

private:
	SsChannel* allocChannel();
	void updateChannel( SsChannel* Channel, SoLoud::handle Handle );
	void freeChannel( SsChannel* Channel );
	
	void internalUpdate();
	
public:
	SoLoud::Soloud* SoLoudCore_;
	std::list< SsChannel* > FreeChannels_;
	std::list< SsChannel* > PendingChannels_;
	std::list< SsChannel* > UsedChannels_;
	std::map< SsChannel*, SsChannelCallback > ChannelDoneCallbacks_;
	std::recursive_mutex ChannelMutex_;

	SysFence WaitFence_;
	
};

#endif

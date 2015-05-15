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
	/**
	 * Create a bus to use for mixing played sources on.
	 * @param Params Bus creation parameters.
	 * @pre SsCore has been created.
	 * @return Return valid bus to use.
	 */
	virtual class SsBus* createBus(
		const class SsBusParams& Params ) = 0;

	/**
	 * Create a filter to process mixed audio.
	 * @param Params Filter creation params.
	 * @pre SsCore has been created.
	 * @return Return valid filter to use.
	 */
	virtual class SsFilter* createFilter( 
		const class SsFilterParams& Params ) = 0;

	/**
	 * Create a source for sound to be streamed from.
	 * @param Params Source creation parameters.
	 * @param FileData File data.
	 * @pre SsCore has been created.
	 * @return valid source to use.
	 */
	virtual class SsSource* createSource( 
		const class SsSourceParams& Params,
		const struct SsSourceFileData* FileData ) = 0;

	/**
	 * Destroy bus resource.
	 * @pre SsCore has been created.
	 * @pre @a Resource is valid.
	 * @post @a Resource destroyed, no longer valid to dereference.
	 */
	virtual void destroyResource( 
		class SsBus* Resource ) = 0;
	
	/**
	 * Destroy filter resource.
	 * @pre SsCore has been created.
	 * @pre @a Resource is valid.
	 * @post @a Resource destroyed, no longer valid to dereference.
	 */
	virtual void destroyResource( 
		class SsFilter* Resource ) = 0;

	/**
	 * Destroy source resource.
	 * @pre SsCore has been created.
	 * @pre @a Resource is valid.
	 * @post @a Resource destroyed, no longer valid to dereference.
	 */
	virtual void destroyResource( 
		class SsSource* Resource ) = 0;

	/**
	 * Play source.
	 * @param Source Source to get sound from.
	 * @param Params Play parameters.
	 * @param DoneCallback Callback to call once audio is done.
	 * @pre SsCore has been created.
	 * @pre @a Source is valid.
	 * @pre @a DoneCallback safe to be called from any thread.
	 * @return Channel sound is to be played on. nullptr if failed to play.
	 */
	virtual class SsChannel* playSource( 
		class SsSource* Source, 
		const class SsChannelParams& Params,
		SsChannelCallback DoneCallback ) = 0;

	/**
	 * Stop channel.
	 * @param Channel Channel to stop playing.
	 * @param ForceFlush True to wait for audio threads to stop sound.
	 * @pre SsCore has been created.
	 * @pre Channel is valid.
	 * @post Channel will stop playing.
	 */
	virtual void stopChannel( 
		class SsChannel* Channel,
		BcBool ForceFlush = BcFalse ) = 0;

	/**
	 * Update channel.
	 * @param Channel Channel to update parameters for.
	 * @param Params New channel parameters.
	 * @pre SsCore has been created.
	 * @pre Channel is valid.
	 * @post Channel has been updated.
	 */
	virtual void updateChannel(
		class SsChannel* Channel,
		const class SsChannelParams& Params ) = 0;

	/**
	 * Set listener.
	 * @param Transform Listener transform local->world.
	 * @param Velocity World space velocity of listener.
	 * @pre SsCore has been created.
	 */
	virtual void setListener( 
		const MaMat4d& Transform, const MaVec3d& Velocity ) = 0;

	/**
	 * Get visualisation data.
	 * @param OutFFT Output vector for FFT data.
	 * @param OutWave Output vector for wave data.
	 * @pre SsCore has been created.
	 * @post @a OutFFT and @a OutWave will be overwritten and refilled with data.
	 */
	virtual void getVisualisationData( std::vector< BcF32 >& OutFFT, std::vector< BcF32 >& OutWave ) = 0;

protected:
	/**
	 * Set channel params on channel object.
	 * Used to enforce that only SsCore implementations can modify it.
	 */
	void setChannelParams( class SsChannel* Channel, const class SsChannelParams& Params );

};

#endif


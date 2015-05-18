/**************************************************************************
*
* File:		Sound/ScnSoundEmitter.h
* Author:	Neil Richardson 
* Ver/Date:	28/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSoundEmitterComponent_H__
#define __ScnSoundEmitterComponent_H__

#include "System/Scene/ScnComponent.h"
#include "System/Scene/Sound/ScnSound.h"

#include "System/Sound/SsChannel.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundEmitterComponentRef
typedef ReObjectRef< class ScnSoundEmitterComponent > ScnSoundEmitterComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnSoundEmitterComponent
class ScnSoundEmitterComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnSoundEmitterComponent, ScnComponent );
	
	ScnSoundEmitterComponent();
	virtual ~ScnSoundEmitterComponent();
		
	/**
	 * Play sound.
	 * @param Sound Sound resource to begin playing.
	 * @param ContinuousUpdate true if sound is to be updated
	 *        whilst it plays (position, pitch, gain, etc).
	 *        false if it is intended to only use current parameters.
	 */
	void play( ScnSoundRef Sound, bool ContinuousUpdate = false );

	/**
	 * Stop all currently playing sounds on emitter.
	 * @param ForceFlush true if this method should wait untl
	 *        the audio threads are done processing, false
	 *        if not.
	 */
	void stopAll( bool ForceFlush = false );	

	/**
	 * Set gain.
	 * @param Gain 0.0f for no sound, 1.0f for normal.
	 * @pre @a Gain >= 0.0f.
	 */
	void setGain( BcF32 Gain );

	/**
	 * Set pitch.
	 * @param Pitch Multiplier of default, so 1.0f for normal, 0.0f for stopped.
	 * @pre @a Pitch is > 0.0f.
	 */
	void setPitch( BcF32 Pitch );

	/**
	 * Set min/max distance.
	 * @param MinDistance Minimum distance for attenuation. See @a SsAttenuationModel.
	 * @param MaxDistance Maximum distance for attenuation. See @a SsAttenuationModel.
	 * @pre @a MinDistance > 0.0f.
	 * @pre @a MaxDistance > @a MinDistance.
	 */
	void setMinMaxDistance( BcF32 MinDistance, BcF32 MaxDistance );

	/**
	 * Set attenuation.
	 * @param AttenuationModel Attenuation model to use. Maps on to SoLoud's.
	 * @param RolloffFactor Rolloff factor for attenuation model.
	 * @pre @a RolloffFactor > 0.0f.
	 */
	void setAttenuation( SsAttenuationModel AttenuationModel, BcF32 RolloffFactor );

private:
	void postUpdate( BcF32 Tick ) override;
	void updateVelocity( BcF32 Tick );
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;
	void onChannelDone( SsChannel* Channel );

	typedef std::map< SsChannel*, ScnSoundRef >	TChannelSoundMap;
	typedef std::vector< SsChannel* > TChannelSoundUpdateList;
	typedef TChannelSoundMap::iterator	TChannelSoundMapIterator;

	TChannelSoundMap ChannelSoundMap_;
	TChannelSoundUpdateList ChannelUpdateList_;
	std::recursive_mutex ChannelSoundMutex_; // TODO: remove later.

	BcF32 Gain_;
	BcF32 Pitch_;
	BcF32 MinDistance_;
	BcF32 MaxDistance_;
	SsAttenuationModel AttenuationModel_;
	BcF32 RolloffFactor_;

	BcF32 VelocityMultiplier_;
	BcF32 VelocitySmoothingAmount_;
	BcF32 MaxVelocity_;

	MaVec3d LastPosition_;
	MaVec3d Position_;
	MaVec3d Velocity_;
	MaVec3d SmoothedVelocity_;
};


#endif



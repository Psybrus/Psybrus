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
		
	void play( ScnSoundRef Sound ) ;
	void stopAll( BcBool ForceFlush = BcFalse );	

	void setGain( BcF32 Gain );
	void setPitch( BcF32 Pitch );

	void update( BcF32 Tick ) override;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

private:
	void onChannelDone( SsChannel* Channel );

	typedef std::map< SsChannel*, ScnSoundRef >	TChannelSoundMap;
	typedef std::vector< SsChannel* > TChannelSoundUpdateList;
	typedef TChannelSoundMap::iterator	TChannelSoundMapIterator;

	TChannelSoundMap ChannelSoundMap_;
	TChannelSoundUpdateList ChannelUpdateList_;
	std::recursive_mutex ChannelSoundMutex_; // TODO: remove later.

	SsChannelParams Params_;
};


#endif



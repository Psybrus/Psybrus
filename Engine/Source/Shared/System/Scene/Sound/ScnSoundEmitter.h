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
	DECLARE_RESOURCE( ScnSoundEmitterComponent, ScnComponent );
	
	virtual void						initialise( const Json::Value& Object );
	
	void								setGain( BcF32 Gain );
	void								setPitch( BcF32 Pitch );

	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	void								play( ScnSoundRef Sound );
	void								stopAll( BcBool ForceFlush = BcFalse );	

	void								onChannelDone( SsChannel* Channel );

private:
	typedef std::map< SsChannel*, ScnSoundRef >	TChannelSoundMap;
	typedef TChannelSoundMap::iterator	TChannelSoundMapIterator;

	TChannelSoundMap					ChannelSoundMap_;
	std::recursive_mutex				ChannelSoundMutex_; // TODO: remove later.

	SsChannelParams						Params_;
};


#endif



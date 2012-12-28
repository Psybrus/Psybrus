/**************************************************************************
*
* File:		ScnSoundEmitter.h
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
#include "System/Scene/ScnSound.h"

#include "System/Sound/SsChannel.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundEmitterComponentRef
typedef CsResourceRef< class ScnSoundEmitterComponent > ScnSoundEmitterComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnSoundEmitterComponent
class ScnSoundEmitterComponent:
	public ScnComponent,
	public SsChannelCallback
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnSoundEmitterComponent );
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
	void								setGain( BcF32 Gain );
	void								setPitch( BcF32 Pitch );

	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );


	void								play( ScnSoundRef Sound );
	void								play( const BcName& Package, const BcName& Name );

	void								stopAll();

protected:
	virtual void						onStarted( SsChannel* pSound );
	virtual void						onPlaying( SsChannel* pSound );
	virtual void						onEnded( SsChannel* pSound );
	
private:
	typedef std::map< SsChannel*, ScnSoundRef >	TChannelSoundMap;
	typedef TChannelSoundMap::iterator	TChannelSoundMapIterator;

	TChannelSoundMap					ChannelSoundMap_;

	BcVec3d								Position_;
	BcF32								Gain_;
	BcF32								Pitch_;
};


#endif



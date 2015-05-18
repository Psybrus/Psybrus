/**************************************************************************
*
* File:		Sound/ScnSoundListenerComponent.h
* Author:	Neil Richardson 
* Ver/Date:	23/04/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSoundListenerComponent_H__
#define __ScnSoundListenerComponent_H__

#include "System/Scene/ScnComponent.h"
#include "System/Scene/Sound/ScnSound.h"

#include "System/Sound/SsChannel.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundListenerComponentRef
typedef ReObjectRef< class ScnSoundListenerComponent > ScnSoundListenerComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnSoundListenerComponent
class ScnSoundListenerComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnSoundListenerComponent, ScnComponent );
	
	ScnSoundListenerComponent();
	virtual ~ScnSoundListenerComponent();

private:	
	void postUpdate( BcF32 Tick ) override;
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	BcF32 VelocityMultiplier_;
	BcF32 VelocitySmoothingAmount_;
	BcF32 MaxVelocity_;
	
	MaVec3d LastPosition_;
	MaVec3d Position_;
	MaVec3d Velocity_;
	MaVec3d SmoothedVelocity_;
};


#endif



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
	
	virtual void postUpdate( BcF32 Tick );
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );
};


#endif



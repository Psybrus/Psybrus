/**************************************************************************
*
* File:		ScnSoundListenerComponent.h
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
#include "System/Scene/ScnSound.h"

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
	DECLARE_RESOURCE( ScnComponent, ScnSoundListenerComponent );
	
	virtual void						initialise( const Json::Value& Object );
	virtual void						postUpdate( BcF32 Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
};


#endif



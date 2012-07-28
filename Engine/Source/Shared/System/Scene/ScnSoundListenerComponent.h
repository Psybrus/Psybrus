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
typedef CsResourceRef< class ScnSoundListenerComponent > ScnSoundListenerComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnSoundListenerComponent
class ScnSoundListenerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnSoundListenerComponent );
	
	virtual void						initialise();
	virtual void						update( BcReal Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
};


#endif



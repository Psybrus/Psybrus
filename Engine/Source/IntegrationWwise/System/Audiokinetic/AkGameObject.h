/**************************************************************************
*
* File:		AkGameObject.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __AkGameObject_H__
#define __AkGameObject_H__

#include "System/Scene/ScnComponent.h"

#include "System/Audiokinetic/AkEvents.h"

//////////////////////////////////////////////////////////////////////////
// AkGameObjectComponentRef
typedef CsResourceRef< class AkGameObjectComponent > AkGameObjectComponentRef;

//////////////////////////////////////////////////////////////////////////
// AkGameObjectComponent
class AkGameObjectComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, AkGameObjectComponent );
	
	virtual void						initialise();
	virtual void						update( BcReal Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	eEvtReturn							onEventPost( EvtID ID, const AkEventPost& Event );
	eEvtReturn							onEventSetRTPC( EvtID ID, const AkEventSetRTPC& Event );

	AkGameObjectID						getGameObjectID();
};


#endif


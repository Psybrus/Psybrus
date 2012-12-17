/**************************************************************************
*
* File:		AkListener.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __AkListener_H__
#define __AkListener_H__

#include "System/Scene/ScnComponent.h"

#include <AK/SoundEngine/Common/AkTypes.h>

//////////////////////////////////////////////////////////////////////////
// AkListenerComponentRef
typedef CsResourceRef< class AkListenerComponent > AkListenerComponentRef;

//////////////////////////////////////////////////////////////////////////
// AkListenerComponent
class AkListenerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, AkListenerComponent );
	
	virtual void						initialise();
	virtual void						initialise( BcU32 ListenerID );
	virtual void						update( BcReal Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	BcU32								ListenerID_;
};


#endif


/**************************************************************************
*
* File:		AkRTPC.h
* Author:	Neil Richardson 
* Ver/Date:
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __AkRTPC_H__
#define __AkRTPC_H__

#include "System/Audiokinetic/AkCore.h"
#include "System/Audiokinetic/AkGameObject.h"


//////////////////////////////////////////////////////////////////////////
// AkRTPCComponentRef
typedef CsResourceRef< class AkRTPCComponent > AkRTPCComponentRef;

//////////////////////////////////////////////////////////////////////////
// AkRTPCComponent
class AkRTPCComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, AkRTPCComponent );
	
	virtual void						initialise();
	virtual void						update( BcReal Tick );
	
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	AkGameObjectComponentRef			GameObject_;
	AkUniqueID							RTPCID_;
	AkReal32							Value_;
	BcBool								Dirty_;

};


#endif

/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Portaudio component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPortaudioComponent_H__
#define __GaPortaudioComponent_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaPortaudioComponent
typedef CsResourceRef< class GaPortaudioComponent > GaPortaudioComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaPortaudioComponent
class GaPortaudioComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPortaudioComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:

};

#endif


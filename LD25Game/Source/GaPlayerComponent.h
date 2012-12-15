/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Player component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPlayerComponent_H__
#define __GaPlayerComponent_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
typedef CsResourceRef< class GaPlayerComponent > GaPlayerComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
class GaPlayerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPlayerComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:

};

#endif


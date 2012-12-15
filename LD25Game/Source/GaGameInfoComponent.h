/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Game info component. Config and stuff.
*		
*
*
* 
**************************************************************************/

#ifndef __GAGAMEINFOCOMPONENT_H__
#define __GAGAMEINFOCOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaGameInfoComponent
typedef CsResourceRef< class GaGameInfoComponent > GaGameInfoComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaGameInfoComponent
class GaGameInfoComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaGameInfoComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:

};

#endif


/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Game state component. Logic.
*		
*
*
* 
**************************************************************************/

#ifndef __GAGAMESTATECOMPONENT_H__
#define __GAGAMESTATECOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
typedef CsResourceRef< class GaGameStateComponent > GaGameStateComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
class GaGameStateComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaGameStateComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:

};

#endif


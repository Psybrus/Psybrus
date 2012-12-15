/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Enemy component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaEnemyComponent_H__
#define __GaEnemyComponent_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
typedef CsResourceRef< class GaEnemyComponent > GaEnemyComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
class GaEnemyComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaEnemyComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:

};

#endif


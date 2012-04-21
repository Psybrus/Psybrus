/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
* Description:
*		Camera component.
*		
*
*
* 
**************************************************************************/

#ifndef __GACAMERACOMPONENT_H__
#define __GACAMERACOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaCameraComponent > GaCameraComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
class GaCameraComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaCameraComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:
	BcReal Ticker_;
};

#endif


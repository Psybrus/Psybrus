/**************************************************************************
*
* File:		GaPaddleComponent.h
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Paddle component.
*		
*
*
* 
**************************************************************************/

#ifndef __GAPADDLECOMPONENT_H__
#define __GAPADDLECOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaPaddleComponent > GaPaddleComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaPaddleComponent
class GaPaddleComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPaddleComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:
	
};

#endif


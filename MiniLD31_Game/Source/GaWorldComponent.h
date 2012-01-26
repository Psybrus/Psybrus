/**************************************************************************
*
* File:		GaSwarmComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#ifndef __GaWorldComponent_H__
#define __GaWorldComponent_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaMainGameState;
typedef CsResourceRef< class GaWorldComponent > GaWorldComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaWorldComponent
class GaWorldComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaWorldComponent );

	virtual void initialise();
	virtual void update( BcReal Tick );
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

public:

	ScnCanvasComponentRef CanvasComponent_;

	BcBool GeneratedNodeGraph_;
};

#endif

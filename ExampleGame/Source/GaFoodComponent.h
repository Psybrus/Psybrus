/**************************************************************************
*
* File:		GaFoodComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Food.
*		
*
*
* 
**************************************************************************/

#ifndef __GaFoodComponent_H__
#define __GaFoodComponent_H__

#include "Psybrus.h"

#include "GaGameComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResourceRef< class GaFoodComponent > GaFoodComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaFoodComponent
class GaFoodComponent:
	public GaGameComponent
{
public:
	DECLARE_RESOURCE( GaGameComponent, GaFoodComponent );
	
	virtual void initialise( const BcVec2d& Position );

	void eat( BcReal Tick );

	virtual BcBool isAlive();
	BcReal getHealthFraction();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasComponentRef Canvas );
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

private:
	BcReal StartingHealth_;
	BcReal Health_;

	ScnMaterialComponentRef MaterialComponent_;
	ScnMaterialComponentRef ShadowMaterialComponent_;
};

#endif

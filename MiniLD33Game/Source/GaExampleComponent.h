/**************************************************************************
*
* File:		GaExampleComponent.h
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#ifndef __GAEXAMPLECOMPONENT_H__
#define __GAEXAMPLECOMPONENT_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaExampleComponent > GaExampleComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaExampleComponent
class GaExampleComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaExampleComponent );

	void								initialise();

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );


private:
	ScnCanvasComponentRef				CanvasComponent_;

	BcReal								Rotation_;
};

#endif


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

#ifndef __GaTitleComponent_H__
#define __GaTitleComponent_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaMainGameState;
typedef CsResourceRef< class GaTitleComponent > GaTitleComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaTitleComponent
class GaTitleComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaTitleComponent );

	virtual void initialise( ScnMaterialRef Material, BcReal Time, BcReal BestTime );
	virtual void update( BcReal Tick );
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

public:
	ScnCanvasComponentRef CanvasComponent_;
	ScnFontComponentRef FontComponent_;

	enum FadeStage
	{
		FS_IN,
		FS_WAIT,
		FS_OUT,
	};

	FadeStage FadeStage_;

	BcReal Fade_;

	BcReal Time_;
	BcReal BestTime_;
};

#endif

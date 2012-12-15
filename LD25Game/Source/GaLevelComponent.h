/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Level component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaLevelComponent_H__
#define __GaLevelComponent_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaLevelComponent
typedef CsResourceRef< class GaLevelComponent > GaLevelComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaLevelComponent
class GaLevelComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaLevelComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
private:
	ScnMaterialComponentRef				LevelMaterial_;
	BcU32								UVScrollingParam_;
};

#endif


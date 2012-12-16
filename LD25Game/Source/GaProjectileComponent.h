/**************************************************************************
*
* File:		GaProjectileComponent.h
* Author:	Neil Richardson 
* Ver/Date:	16/12/2012
* Description:
*		Projectile component.
*		
*
*
* 
**************************************************************************/

#ifndef __GAPROJECTILECOMPONENT_H__
#define __GAPROJECTILECOMPONENT_H__

#include "Psybrus.h"

#include "GaEvents.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaProjectileComponent > GaProjectileComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaProjectileComponent
class GaProjectileComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaProjectileComponent );

	void								initialise( const Json::Value& Object );
	
	void								setPositionVelocity( const BcVec3d& Position, const BcVec3d& Velocity );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

private:
	BcVec3d								Position_;
	BcVec3d								Velocity_;

	BcVec3d								AngularVelocity_;
	BcVec3d								Rotation_;
};

#endif

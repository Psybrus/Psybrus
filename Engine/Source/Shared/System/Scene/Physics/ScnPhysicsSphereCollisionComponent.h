/**************************************************************************
*
* File:		ScnPhysicsSphereCollisionComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsSphereCollisionComponent_H__
#define __ScnPhysicsSphereCollisionComponent_H__

#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsSphereCollisionComponent
class ScnPhysicsSphereCollisionComponent:
	public ScnPhysicsCollisionComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsSphereCollisionComponent, ScnPhysicsCollisionComponent );
	
	ScnPhysicsSphereCollisionComponent();
	virtual ~ScnPhysicsSphereCollisionComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

protected:
	BcF32 Radius_;
};

#endif

/**************************************************************************
*
* File:		ScnPhysicsBoxCollisionComponent.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsBoxCollisionComponent_H__
#define __ScnPhysicsBoxCollisionComponent_H__

#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsBoxCollisionComponent
class ScnPhysicsBoxCollisionComponent:
	public ScnPhysicsCollisionComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsBoxCollisionComponent, ScnPhysicsCollisionComponent );
	
	ScnPhysicsBoxCollisionComponent();
	virtual ~ScnPhysicsBoxCollisionComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

protected:
	MaVec3d Size_;
	BcF32 Margin_;

};

#endif

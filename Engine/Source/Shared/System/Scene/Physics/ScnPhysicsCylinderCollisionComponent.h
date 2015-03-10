/**************************************************************************
*
* File:		ScnPhysicsCylinderCollisionComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsCylinderCollisionComponent_H__
#define __ScnPhysicsCylinderCollisionComponent_H__

#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsCylinderCollisionComponent
class ScnPhysicsCylinderCollisionComponent:
	public ScnPhysicsCollisionComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsCylinderCollisionComponent, ScnPhysicsCollisionComponent );
	
	ScnPhysicsCylinderCollisionComponent();
	virtual ~ScnPhysicsCylinderCollisionComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

protected:
	MaVec3d Size_;
	BcF32 Margin_;

};

#endif

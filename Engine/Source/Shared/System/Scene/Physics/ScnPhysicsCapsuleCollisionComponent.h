/**************************************************************************
*
* File:		ScnPhysicsCapsuleCollisionComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsCapsuleCollisionComponent_H__
#define __ScnPhysicsCapsuleCollisionComponent_H__

#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsCapsuleCollisionComponent
class ScnPhysicsCapsuleCollisionComponent:
	public ScnPhysicsCollisionComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsCapsuleCollisionComponent, ScnPhysicsCollisionComponent );
	
	ScnPhysicsCapsuleCollisionComponent();
	virtual ~ScnPhysicsCapsuleCollisionComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

protected:
	BcF32 Radius_;
	BcF32 Height_;
	BcF32 Margin_;

};

#endif

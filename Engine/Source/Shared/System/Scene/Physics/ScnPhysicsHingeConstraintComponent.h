/**************************************************************************
*
* File:		ScnPhysicsHingeConstraintComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsHingeConstraintComponent_H__
#define __ScnPhysicsHingeConstraintComponent_H__

#include "System/Scene/Physics/ScnPhysicsConstraintComponent.h"
#include "System/Scene/Physics/ScnPhysicsRigidBodyComponent.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsHingeConstraintComponent
class ScnPhysicsHingeConstraintComponent:
	public ScnPhysicsConstraintComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsHingeConstraintComponent, ScnPhysicsConstraintComponent );
	
	ScnPhysicsHingeConstraintComponent();
	virtual ~ScnPhysicsHingeConstraintComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

protected:
	class ScnPhysicsRigidBodyComponent* RigidBodyA_;
	class ScnPhysicsRigidBodyComponent* RigidBodyB_;

	BcBool UseReferenceFrameA_;
	MaMat4d FrameA_;
	MaMat4d FrameB_;
};

#endif

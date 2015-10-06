/**************************************************************************
*
* File:		ScnPhysicsConstraintComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsConstraintComponent__
#define __ScnPhysicsConstraintComponent__

#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsConstraintComponent
class ScnPhysicsConstraintComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsConstraintComponent, ScnComponent );
	
	ScnPhysicsConstraintComponent();
	virtual ~ScnPhysicsConstraintComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	class btTypedConstraint* getConstraint();

protected:
	class ScnPhysicsWorldComponent* World_;

	class btTypedConstraint* Constraint_;
};

#endif

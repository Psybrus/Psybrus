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

	virtual void initialise();

	void onAttach( ScnEntityWeakRef Parent );
	void onDetach( ScnEntityWeakRef Parent );

	class btTypedConstraint* getConstraint();

protected:
	class ScnPhysicsWorldComponent* World_;

	class btTypedConstraint* Constraint_;
};

#endif

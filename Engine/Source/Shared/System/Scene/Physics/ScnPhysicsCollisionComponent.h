/**************************************************************************
*
* File:		ScnPhysicsCollisionShape.h
* Author:	Neil Richardson 
* Ver/Date:	20/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsCollisionShape_H__
#define __ScnPhysicsCollisionShape_H__

#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsCollisionComponent
class ScnPhysicsCollisionComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsCollisionComponent, ScnComponent );
	
	ScnPhysicsCollisionComponent();
	virtual ~ScnPhysicsCollisionComponent();

	virtual void initialise();

	class btCollisionShape* getCollisionShape();


protected:
	class btCollisionShape* CollisionShape_;
};

#endif

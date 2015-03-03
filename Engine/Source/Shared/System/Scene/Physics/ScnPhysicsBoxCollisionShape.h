/**************************************************************************
*
* File:		ScnPhysicsBoxCollisionShape.h
* Author:	Neil Richardson 
* Ver/Date:	25/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsBoxCollisionShape_H__
#define __ScnPhysicsBoxCollisionShape_H__

#include "System/Scene/Physics/ScnPhysicsCollisionShape.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsBoxCollisionShape
class ScnPhysicsBoxCollisionShape:
	public ScnPhysicsCollisionShape
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsBoxCollisionShape, ScnPhysicsCollisionShape );
	
	ScnPhysicsBoxCollisionShape();
	virtual ~ScnPhysicsBoxCollisionShape();
	
	virtual void create();
	virtual void destroy();

protected:
	
};

#endif

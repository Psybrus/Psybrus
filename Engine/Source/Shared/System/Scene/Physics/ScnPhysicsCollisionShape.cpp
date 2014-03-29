/**************************************************************************
*
* File:		ScnPhysicsCollisionShape.cpp
* Author:	Neil Richardson 
* Ver/Date:	20/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsCollisionShape.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnPhysicsCollisionShape );

BCREFLECTION_EMPTY_REGISTER( ScnPhysicsCollisionShape );
/*
BCREFLECTION_DERIVED_BEGIN( CsResource, ScnPhysicsCollisionShape )
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPhysicsCollisionShape::initialise()
{
	CollisionShape_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnPhysicsCollisionShape::create()
{
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnPhysicsCollisionShape::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getCollisionShape
//virtual
class btCollisionShape* ScnPhysicsCollisionShape::getCollisionShape()
{
	return CollisionShape_;
}
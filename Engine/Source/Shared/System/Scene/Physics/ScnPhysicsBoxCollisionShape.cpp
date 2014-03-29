/**************************************************************************
*
* File:		ScnPhysicsBoxCollisionShape.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsBoxCollisionShape.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnPhysicsBoxCollisionShape );

BCREFLECTION_EMPTY_REGISTER( ScnPhysicsBoxCollisionShape );
/*
BCREFLECTION_DERIVED_BEGIN( CsResource, ScnPhysicsBoxCollisionShape )
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPhysicsBoxCollisionShape::initialise()
{
	CollisionShape_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnPhysicsBoxCollisionShape::create()
{
	ScnPhysicsCollisionShape::create();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnPhysicsBoxCollisionShape::destroy()
{
	ScnPhysicsCollisionShape::destroy();
}


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
REFLECTION_DEFINE_DERIVED( ScnPhysicsBoxCollisionShape );

void ScnPhysicsBoxCollisionShape::StaticRegisterClass()
{
	ReRegisterClass< ScnPhysicsBoxCollisionShape, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsBoxCollisionShape::ScnPhysicsBoxCollisionShape()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsBoxCollisionShape::~ScnPhysicsBoxCollisionShape()
{
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


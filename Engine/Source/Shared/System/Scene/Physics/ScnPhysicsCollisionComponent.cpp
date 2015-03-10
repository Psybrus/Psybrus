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

#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsCollisionComponent );

void ScnPhysicsCollisionComponent::StaticRegisterClass()
{
	ReRegisterClass< ScnPhysicsCollisionComponent, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsCollisionComponent::ScnPhysicsCollisionComponent():
	CollisionShape_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsCollisionComponent::~ScnPhysicsCollisionComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPhysicsCollisionComponent::initialise()
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// getCollisionShape
//virtual
class btCollisionShape* ScnPhysicsCollisionComponent::getCollisionShape()
{
	return CollisionShape_;
}

/**************************************************************************
*
* File:		ScnPhysicsCollisionShape.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysics.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsCollisionComponent );

void ScnPhysicsCollisionComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "LocalScaling_", &ScnPhysicsCollisionComponent::LocalScaling_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsCollisionComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsCollisionComponent::ScnPhysicsCollisionComponent():
	LocalScaling_( 1.0f, 1.0f, 1.0f ),
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
// setLocalScaling
void ScnPhysicsCollisionComponent::setLocalScaling( const MaVec3d& LocalScaling )
{
	LocalScaling_ = LocalScaling;
	if( CollisionShape_ != nullptr )
	{
		CollisionShape_->setLocalScaling( ScnPhysicsToBullet( LocalScaling_ ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// getCollisionShape
//virtual
class btCollisionShape* ScnPhysicsCollisionComponent::getCollisionShape()
{
	return CollisionShape_;
}

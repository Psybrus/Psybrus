/**************************************************************************
*
* File:		ScnPhysicsSphereCollisionComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsSphereCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysics.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsSphereCollisionComponent );

void ScnPhysicsSphereCollisionComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Radius_", &ScnPhysicsSphereCollisionComponent::Radius_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsSphereCollisionComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsSphereCollisionComponent::ScnPhysicsSphereCollisionComponent():
	Radius_( 1.0f )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsSphereCollisionComponent::~ScnPhysicsSphereCollisionComponent()
{
	delete CollisionShape_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnPhysicsSphereCollisionComponent::onAttach( ScnEntityWeakRef Parent )
{
	CollisionShape_ = new btSphereShape( Radius_ );
	CollisionShape_->setLocalScaling( ScnPhysicsToBullet( LocalScaling_ ) ); // move to base
	CollisionShape_->setUserPointer( this );
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnPhysicsSphereCollisionComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

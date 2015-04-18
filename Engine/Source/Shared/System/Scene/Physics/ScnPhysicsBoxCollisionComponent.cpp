/**************************************************************************
*
* File:		ScnPhysicsBoxCollisionComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsBoxCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysics.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsBoxCollisionComponent );

void ScnPhysicsBoxCollisionComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Size_", &ScnPhysicsBoxCollisionComponent::Size_, bcRFF_IMPORTER ),
		new ReField( "Margin_", &ScnPhysicsBoxCollisionComponent::Margin_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsBoxCollisionComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsBoxCollisionComponent::ScnPhysicsBoxCollisionComponent():
	Size_( 1.0f, 1.0f, 1.0f ),
	Margin_( 0.0f )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsBoxCollisionComponent::~ScnPhysicsBoxCollisionComponent()
{
	delete CollisionShape_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnPhysicsBoxCollisionComponent::onAttach( ScnEntityWeakRef Parent )
{
	CollisionShape_ = new btBoxShape( ScnPhysicsToBullet( Size_ * 0.5f ) );
	CollisionShape_->setLocalScaling( ScnPhysicsToBullet( LocalScaling_ ) );
	CollisionShape_->setMargin( Margin_ );
	CollisionShape_->setUserPointer( this );
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnPhysicsBoxCollisionComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

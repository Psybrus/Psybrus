/**************************************************************************
*
* File:		ScnPhysicsCapsuleCollisionComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsCapsuleCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysics.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsCapsuleCollisionComponent );

void ScnPhysicsCapsuleCollisionComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Radius_", &ScnPhysicsCapsuleCollisionComponent::Radius_, bcRFF_IMPORTER ),
		new ReField( "Height_", &ScnPhysicsCapsuleCollisionComponent::Height_, bcRFF_IMPORTER ),
		new ReField( "Margin_", &ScnPhysicsCapsuleCollisionComponent::Margin_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsCapsuleCollisionComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsCapsuleCollisionComponent::ScnPhysicsCapsuleCollisionComponent():
	Radius_( 1.0f ),
	Height_( 1.0f ),
	Margin_( 0.0f )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsCapsuleCollisionComponent::~ScnPhysicsCapsuleCollisionComponent()
{
	delete CollisionShape_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnPhysicsCapsuleCollisionComponent::onAttach( ScnEntityWeakRef Parent )
{
	CollisionShape_ = new btCapsuleShape( Radius_, Height_ );
	CollisionShape_->setLocalScaling( ScnPhysicsToBullet( LocalScaling_ ) ); // move to base
	CollisionShape_->setMargin( Margin_ );
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnPhysicsCapsuleCollisionComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

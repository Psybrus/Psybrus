/**************************************************************************
*
* File:		ScnPhysicsCylinderCollisionComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsCylinderCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysics.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsCylinderCollisionComponent );

void ScnPhysicsCylinderCollisionComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Size_", &ScnPhysicsCylinderCollisionComponent::Size_, bcRFF_IMPORTER ),
		new ReField( "Margin_", &ScnPhysicsCylinderCollisionComponent::Margin_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsCylinderCollisionComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsCylinderCollisionComponent::ScnPhysicsCylinderCollisionComponent():
	Size_( 1.0f, 1.0f, 1.0f ),
	Margin_( 0.0f )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsCylinderCollisionComponent::~ScnPhysicsCylinderCollisionComponent()
{
	delete CollisionShape_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnPhysicsCylinderCollisionComponent::onAttach( ScnEntityWeakRef Parent )
{
	CollisionShape_ = new btCylinderShape( ScnPhysicsToBullet( Size_ * 0.5f ) );
	CollisionShape_->setLocalScaling( ScnPhysicsToBullet( LocalScaling_ ) ); // move to base
	CollisionShape_->setMargin( Margin_ );
	CollisionShape_->setUserPointer( this );
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnPhysicsCylinderCollisionComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

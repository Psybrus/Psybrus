/**************************************************************************
*
* File:		ScnPhysicsMeshCollisionComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsMeshCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysicsMesh.h"
#include "System/Scene/Physics/ScnPhysics.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsMeshCollisionComponent );

void ScnPhysicsMeshCollisionComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Mesh_", &ScnPhysicsMeshCollisionComponent::Mesh_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Size_", &ScnPhysicsMeshCollisionComponent::Size_, bcRFF_IMPORTER ),
		new ReField( "Margin_", &ScnPhysicsMeshCollisionComponent::Margin_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsMeshCollisionComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsMeshCollisionComponent::ScnPhysicsMeshCollisionComponent():
	Mesh_( nullptr ),
	Size_( 1.0f, 1.0f, 1.0f ),
	Margin_( 0.0f )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsMeshCollisionComponent::~ScnPhysicsMeshCollisionComponent()
{
	delete CollisionShape_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnPhysicsMeshCollisionComponent::onAttach( ScnEntityWeakRef Parent )
{
	CollisionShape_ = Mesh_->createCollisionShape();
	CollisionShape_->setLocalScaling( ScnPhysicsToBullet( LocalScaling_ ) ); // move to base
	CollisionShape_->setMargin( Margin_ );
	CollisionShape_->setUserPointer( this );
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnPhysicsMeshCollisionComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

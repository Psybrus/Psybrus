/**************************************************************************
*
* File:		ScnPhysicsHingeConstraintComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsHingeConstraintComponent.h"
#include "System/Scene/Physics/ScnPhysics.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsHingeConstraintComponent );

void ScnPhysicsHingeConstraintComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "RigidBodyA_", &ScnPhysicsHingeConstraintComponent::RigidBodyA_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "RigidBodyB_", &ScnPhysicsHingeConstraintComponent::RigidBodyB_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "UseReferenceFrameA_", &ScnPhysicsHingeConstraintComponent::UseReferenceFrameA_, bcRFF_IMPORTER ),
		new ReField( "FrameA_", &ScnPhysicsHingeConstraintComponent::FrameA_, bcRFF_IMPORTER ),
		new ReField( "FrameB_", &ScnPhysicsHingeConstraintComponent::FrameB_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsHingeConstraintComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsHingeConstraintComponent::ScnPhysicsHingeConstraintComponent():
	RigidBodyA_( nullptr ),
	RigidBodyB_( nullptr ),
	UseReferenceFrameA_( BcFalse ),
	FrameA_(),
	FrameB_()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsHingeConstraintComponent::~ScnPhysicsHingeConstraintComponent()
{
	delete Constraint_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnPhysicsHingeConstraintComponent::onAttach( ScnEntityWeakRef Parent )
{
	if( RigidBodyA_ != nullptr && RigidBodyB_ != nullptr )
	{
		Constraint_ = new btHingeConstraint(
			*RigidBodyA_->getRigidBody(),
			*RigidBodyB_->getRigidBody(),
			ScnPhysicsToBullet( FrameA_ ),
			ScnPhysicsToBullet( FrameB_ ),
			UseReferenceFrameA_ );
	}
	else if( RigidBodyA_ != nullptr && RigidBodyB_ == nullptr )
	{
		Constraint_ = new btHingeConstraint(
			*RigidBodyA_->getRigidBody(),
			ScnPhysicsToBullet( FrameA_ ),
			UseReferenceFrameA_ );
	}
	else
	{
		RigidBodyA_ = getComponentByType< ScnPhysicsRigidBodyComponent >();
		if( RigidBodyA_ != nullptr )
		{
			Constraint_ = new btHingeConstraint(
				*RigidBodyA_->getRigidBody(),
				ScnPhysicsToBullet( FrameA_ ),
				UseReferenceFrameA_ );
		}
	}

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnPhysicsHingeConstraintComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

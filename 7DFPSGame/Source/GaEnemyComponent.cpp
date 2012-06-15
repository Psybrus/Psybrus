/**************************************************************************
*
* File:		GaEnemyComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		Enemy component.
*		
*
*
* 
**************************************************************************/

#include "GaEnemyComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaEnemyComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaEnemyComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	PulseTimer_ = 0.0f;

	IsTargetting_ = BcFalse;
	TargetTimer_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
//virtual
void GaEnemyComponent::update( BcReal Tick )
{
	// Find move vector.
	BcReal MoveSpeed = 2.0f;
	BcVec3d MoveVector;

	PulseTimer_ += Tick;

	// Set direction and handle if we need to change direction.
	BcVec3d Position = getParentEntity()->getPosition();
	MoveVector = Direction_;

	BcBSPPointInfo BSPPointInfo;
	if( BSP_->lineIntersection( Position, Position + Direction_ * 256.0f, &BSPPointInfo ) )
	{
		if( BSPPointInfo.Distance_ < 1.0f )
		{
			// New direction.
			Direction_ = Direction_.reflect( BSPPointInfo.Plane_.normal() );

			// Belt & braces: it will become denormalised over time.
			Direction_.normalise();
			
			// Clamp position (this is to prevent wandering, if angles aren't sitting on an integer, expect weirdness).
			Position = BcVec3d( BcRound( Position.x() ), BcRound( Position.y() ), BcRound( Position.z() ) );

		}
	}

	BcReal PulseTime = 2.0f;

	if( BSP_->canSeePlayer( getParentEntity()->getPosition() ) )
	{
		PulseTime = 0.5f;
		IsTargetting_ = BcTrue;
		TargetTimer_ -= Tick;
	}
	else
	{
		IsTargetting_ = BcFalse;
		TargetTimer_ = 3.0f;
	}

	// Pulse.
	if( PulseTimer_ > PulseTime )
	{
		Pressure_->setSample( Position, 0.5f );
		PulseTimer_ = 0.0f;
	}

	// Shoot player.
	if( IsTargetting_ && TargetTimer_ < 0.0f )
	{
		int a = 0; ++a;
	}
	
	// Set the move.
	if( !IsTargetting_ )
	{
		BcVec3d AppliedMoveVector = MoveVector;
		AppliedMoveVector.z( 0.0f );
		AppliedMoveVector = AppliedMoveVector.normal() * MoveSpeed;
		Pawn_->setMove( AppliedMoveVector );
	}
	else
	{
		Pawn_->setMove( BcVec3d( 0.0f, 0.0f, 0.0f ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaEnemyComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );

	// Cache world stuff we need.
	ScnEntityRef WorldEntity = ScnCore::pImpl()->findEntity( "WorldEntity_0" );
	Canvas_ = WorldEntity->getComponentByType< ScnCanvasComponent >( 0 );
	BSP_ = WorldEntity->getComponentByType< GaWorldBSPComponent >( 0 );
	Pressure_ = WorldEntity->getComponentByType< GaWorldPressureComponent >( 0 );

	// Grab pawn.
	Pawn_ = Parent->getComponentByType< GaPawnComponent >( 0 );

	// Get direction.
	Direction_ = findLongestDirection();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaEnemyComponent::onDetach( ScnEntityWeakRef Parent )
{
	Canvas_ = NULL;
	BSP_ = NULL;
	Pressure_ = NULL;
	Pawn_ = NULL;

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// findLongestDirection
BcVec3d GaEnemyComponent::findLongestDirection()
{
	// Ray cast in the 4 directions to determine path.
	BcReal LongestDistance = 0.0f;
	BcVec3d LongestDirection( 0.0f, 0.0f, 0.0f );
	BcVec3d Directions[] =
	{
		BcVec3d(  1.0f,  0.0f,  0.0f ),
		BcVec3d( -1.0f,  0.0f,  0.0f ),
		BcVec3d(  0.0f,  1.0f,  0.0f ),
		BcVec3d(  0.0f, -1.0f,  0.0f ),
	};

	BcVec3d Position = getParentEntity()->getPosition();

	BcBSPPointInfo BSPPointInfo;
	for( BcU32 Idx = 0; Idx < 4; ++Idx )
	{
		if( BSP_->lineIntersection( Position, Position + Directions[ Idx ] * 256.0f, &BSPPointInfo ) )
		{
			if( BSPPointInfo.Distance_ > LongestDistance )
			{
				LongestDistance = BSPPointInfo.Distance_;
				LongestDirection = Directions[ Idx ];
			}
		}
	}
	
	BcPrintf( "GaEnemyComponent: Got direction [%.1f, %.1f, %.f1]\n", LongestDirection.x(), LongestDirection.y(), LongestDirection.z() );

	return LongestDirection;
}

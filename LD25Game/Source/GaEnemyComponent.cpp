/**************************************************************************
*
* File:		GaEnemyComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Enemy component.
*		
*
*
* 
**************************************************************************/

#include "GaEnemyComponent.h"

#include "GaProjectileComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaEnemyComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaEnemyComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Position_ = BcVec3d( 0.0f, 5.0f, 0.0f );
	TargetPosition_ = Position_;

	MaxHealth_ = 30.0f;
	Health_ = MaxHealth_;
	EventHealthAmount_ = 10.0f;
	NextEventHealth_ = Health_ - EventHealthAmount_;
	TurnTimer_ = 0.0f;

	ProjectileTime_ = 2.0f;
	ProjectileTimer_ = ProjectileTime_;

	GameState_ = GS_EVADING;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaEnemyComponent::update( BcReal Tick )
{
	Super::update( Tick );

	if( Health_ > 0.0f )
	{
		switch( GameState_ )
		{
		case GS_EVADING:
			{
				// On an event whilst evading, switch to turning.
				if( Health_ < NextEventHealth_ )
				{
					NextEventHealth_ = NextEventHealth_ - EventHealthAmount_;
					GameState_ = GS_TURNING;
				}
			}
			break;

		case GS_TURNING:
			{
				BcReal Start = 0.0f;
				BcReal End = BcPI;
				TurnTimer_ += Tick;
				Rotation_.z( BcLerp( Start, End, BcSmoothStep( BcSmoothStep( TurnTimer_  ) ) ) );
				if( TurnTimer_ > 1.0f )
				{
					Rotation_.z( BcPI );
					GameState_ = GS_FACING;
					TurnTimer_ = 0.0f;
				}
			}
			break;

		case GS_FACING:
			{
				// On an event whilst evading, switch to turning.
				if( Health_ < NextEventHealth_ )
				{
					NextEventHealth_ = NextEventHealth_ - EventHealthAmount_;
					GameState_ = GS_TURNING_BACK;
				}

				ProjectileTimer_ -= Tick;

				if( ProjectileTimer_ < 0.0f )
				{
					ProjectileTimer_ += ProjectileTime_;

					// Fire projectiles.
					BcVec3d Positions[ 3 ] = 
					{
						Position_,
						Position_,
						Position_,
					};

					BcVec3d Velocities[ 3 ] = 
					{
						BcVec3d( -0.5f, -1.0f, 0.0f ).normal() * 2.0f,
						BcVec3d( -0.0f, -1.0f, 0.0f ).normal() * 2.0f,
						BcVec3d(  0.5f, -1.0f, 0.0f ).normal() * 2.0f,
					};

					for( BcU32 Idx = 0; Idx < 3; ++Idx )
					{
						ScnEntityRef Entity = ScnCore::pImpl()->createEntity( "default", "RockEntity" );
						GaProjectileComponentRef Component = Entity->getComponentByType< GaProjectileComponent >( 0 );
						Component->setPositionVelocity( Positions[ Idx ], Velocities[ Idx ] );
						getParentEntity()->getParentEntity()->attach( Entity );
					}
				}
			}
			break;

		case GS_TURNING_BACK:
			{
				BcReal Start = BcPI;
				BcReal End = 0.0f;
				TurnTimer_ += Tick;
				Rotation_.z( BcLerp( Start, End, BcSmoothStep( BcSmoothStep( TurnTimer_ ) ) ) );
				if( TurnTimer_ > 1.0f )
				{
					Rotation_.z( 0.0f );
					GameState_ = GS_EVADING;

					TurnTimer_ = 0.0f;
				}
			}
			break;

		case GS_ATTACKING:
			{
			}
			break;
		}

		// Movement.
		Position_ = ( Position_ * 0.99f ) + ( TargetPosition_ * 0.01f );
	}

	// Hacky floaty.
	static BcReal Ticker = 0.0f;
	Ticker += Tick;
	BcMat4d Matrix;
	BcVec3d FinalPosition = Position_ + BcVec3d( BcSin( Ticker ) * 0.05f, 0.0f, BcCos( Ticker * 0.7f ) * 0.05f );
	Matrix.rotation( BcVec3d( BcSin( Ticker ) * 0.01f, BcCos( Ticker * 0.9f ) * 0.02f, 0.0f ) + Rotation_ );
	Matrix.translation( FinalPosition );
	getParentEntity()->setMatrix( Matrix );

	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
	BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
	BcReal AspectRatio = HW / HH;

	BcReal Width = HW;
	BcReal HealthSize = BcClamp( Health_ / MaxHealth_, 0.0f, 1.0f ) * Width;
	Canvas_->setMaterialComponent( DefaultMaterial_ );
	Canvas_->drawSpriteCentered( BcVec2d( 0.0f, -HH + 32.0f ), BcVec2d( Width + 4.0f, 18.0f ), 0, RsColour::BLACK, 9 );
	Canvas_->drawLineBoxCentered( BcVec2d( 0.0f, -HH + 32.0f ), BcVec2d( Width + 4.0f, 18.0f ), RsColour::WHITE, 10 );
	Canvas_->drawSpriteCentered( BcVec2d( 0.0f, -HH + 32.0f ), BcVec2d( HealthSize, 14.0f ), 0, RsColour::RED, 11 );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaEnemyComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );

	GaEventShoot::Delegate OnPlayerShoot = GaEventShoot::Delegate::bind< GaEnemyComponent, &GaEnemyComponent::onPlayerShoot >( this );
	getParentEntity()->getParentEntity()->subscribe( gaEVT_PLAYER_SHOOT, OnPlayerShoot );

	GaEventPosition::Delegate OnPlayerPosition = GaEventPosition::Delegate::bind< GaEnemyComponent, &GaEnemyComponent::onPlayerPosition >( this );
	getParentEntity()->getParentEntity()->subscribe( gaEVT_PLAYER_POSITION, OnPlayerPosition );

	// Grab default material.
	DefaultMaterial_ = Parent->getComponentByType< ScnMaterialComponent >( 0 );

	// Grab the canvas.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );

	// Grab particle system (want solid one).
	GameParticles_ = Parent->getParentEntity()->getComponentByType< ScnParticleSystemComponent >( 1 );

}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaEnemyComponent::onDetach( ScnEntityWeakRef Parent )
{
	if( getParentEntity().isValid() && getParentEntity()->getParentEntity().isValid() )
	{
		getParentEntity()->getParentEntity()->unsubscribeAll( this );
	}

	Canvas_ = NULL;
	GameParticles_=  NULL;
	DefaultMaterial_ = NULL;

	// Don't forget to detach!
	Super::onDetach( Parent );
}


//////////////////////////////////////////////////////////////////////////
// particleHit
void GaEnemyComponent::particleHit( BcVec3d Position )
{
	if( GameParticles_.isValid() )
	{
		for( BcU32 Idx = 0; Idx < 64; ++Idx )
		{
			ScnParticle* pParticle = NULL;
			if( GameParticles_->allocParticle( pParticle ) )
			{
				pParticle->Position_ = Position;
				pParticle->Velocity_ = BcVec3d( 0.0f, -5.1f, 0.0f ) + ( BcRandom::Global.randVec3().normal() * BcRandom::Global.randReal() ) ;
				pParticle->Acceleration_ = BcVec3d( 0.0f, -0.1f, 0.0f );

				pParticle->Scale_ = BcVec2d( 0.1f, 0.1f );
				pParticle->MinScale_ = BcVec2d( 0.1f, 0.1f );
				pParticle->MaxScale_ = BcVec2d( 0.9f, 0.9f );

				pParticle->Rotation_ = BcRandom::Global.randReal();
				pParticle->RotationMultiplier_ = BcRandom::Global.randReal();

				pParticle->Colour_ = RsColour( 1.0f, 0.01f, 0.01f, 1.0f );
				pParticle->MinColour_ = RsColour( 1.0f, 0.01f, 0.01f, 1.0f );
				pParticle->MaxColour_ = RsColour( 0.0f, 0.0f, 0.0f, 0.0f );

				pParticle->TextureIndex_ = 2;
				pParticle->CurrentTime_ = 0.0f;
				pParticle->MaxTime_ = 0.9f;
				pParticle->Alive_ = BcTrue;		
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onPlayerShoot
eEvtReturn  GaEnemyComponent::onPlayerShoot( EvtID ID, const GaEventShoot& Event )
{
	// DO DAMAGE.
	BcReal PlayerDistance = BcAbs( Position_.x() - Event.Position_.x() );

	BcVec3d BloodPosition = Position_;
	BloodPosition.x( Event.Position_.x() );

	if( PlayerDistance < 0.2f )
	{
		particleHit( BloodPosition );

		Health_ -= 5.0f;

		if( Health_ <= 0.0f )
		{
			getParentEntity()->getParentEntity()->publish( gaEVT_ENEMY_DIE, EvtNullEvent() );			
		}
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onPlayerPosition
eEvtReturn  GaEnemyComponent::onPlayerPosition( EvtID ID, const GaEventPosition& Event )
{
	const BcReal Width = 1.5f;
	BcVec3d LeftMost( -Width, 0.0f, 0.0f );
	BcVec3d RightMost( Width, 0.0f, 0.0f );

	BcReal LeftMostDistance = BcAbs( LeftMost.x() - Event.Position_.x() );
	BcReal RightMostDistance = BcAbs( RightMost.x() - Event.Position_.x() );
	BcReal PlayerDistance = BcAbs( Position_.x() - Event.Position_.x() );
	
	if( PlayerDistance < 1.0f )
	{
		if( LeftMostDistance > RightMostDistance )
		{
			TargetPosition_.x( LeftMost.x() );
		}
		else
		{
			TargetPosition_.x( RightMost.x() );
		}
	}

	return evtRET_PASS;
}

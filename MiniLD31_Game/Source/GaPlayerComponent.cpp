/**************************************************************************
*
* File:		GaPlayerComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	7
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaPlayerComponent.h"

#include "GaTopState.h"

#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaPlayerComponent )

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaPlayerComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaPlayerComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaPlayerComponent::initialise()
{
	Rotation_=  0.0f;
	MoveU_ = BcFalse;
	MoveD_ = BcFalse;
	MoveL_ = BcFalse;
	MoveR_ = BcFalse;
	Running_ = BcFalse;

	WaterDropTimer_ = 0.0f;

	// Find the world entity.
	ScnEntityRef WorldEntity;
	if( CsCore::pImpl()->findResource( "WorldEntity_0", WorldEntity ) )
	{
		// Get it's material component.
		WorldMaterialComponent_ = WorldEntity->getComponent( 0 );
		BcAssert( WorldMaterialComponent_.isValid() );
		InverseDrawDistanceParam_= WorldMaterialComponent_->findParameter( "uInverseDrawDistance" );
		NoiseTextureParam_= WorldMaterialComponent_->findParameter( "aNoiseTex" );

		MinDrawDistance_ = 2.0f;
		MaxDrawDistance_ = 16.0f;

		ActualDrawDistance_ = 0.0f;
		TargetDrawDistance_ = MaxDrawDistance_;
	}

	CsCore::pImpl()->findResource( "waterdrop0", WaterDropSound_ );
	CsCore::pImpl()->findResource( "footstepslow", FootStepSlowSound_ );
	CsCore::pImpl()->findResource( "footstepfast", FootStepFastSound_ );
	CsCore::pImpl()->findResource( "ambience0", Ambience0Sound_ );
	CsCore::pImpl()->findResource( "ambience1", Ambience1Sound_ );
	CsCore::pImpl()->findResource( "jump", JumpSound_ );
	CsCore::pImpl()->findResource( "land", LandSound_ );

	CsCore::pImpl()->findResource( "noise0", NoiseTexture_[0] );
	CsCore::pImpl()->findResource( "noise1", NoiseTexture_[1] );
	CsCore::pImpl()->findResource( "noise2", NoiseTexture_[2] );
	CsCore::pImpl()->findResource( "noise3", NoiseTexture_[3] );

	CsCore::pImpl()->createResource( BcName::INVALID, LocalEmitter_ );
	CsCore::pImpl()->createResource( BcName::INVALID, Ambience0Emitter_ );
	CsCore::pImpl()->createResource( BcName::INVALID, Ambience1Emitter_ );

	MoveTimer_ = 2.0f;
	TargetMoveSpeed_ = 8.0f;

	AmbienceVolume_ = 0.0f;
	Ambience0Emitter_->setGain( AmbienceVolume_ );
	Ambience1Emitter_->setGain( AmbienceVolume_ );

	Ambience0Emitter_->play( Ambience0Sound_ );
	Ambience1Emitter_->play( Ambience1Sound_ );

	WasOnGround_ = BcFalse;

	CurrNoiseTexture_ = 0;
	NoiseTextureTimer_ = 0.0f;

	CameraShakeDepth_ = 0.0f;
	CameraShakeTimer_ = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaPlayerComponent::update( BcReal Tick )
{
	//
	LocalEmitter_->setGain( Running_ ? 0.5f : 0.4f );

	// Set transform from ghost object.
	const btTransform& PhysicsTransform = pGhostObject_->getWorldTransform();
	const btVector3& Origin = PhysicsTransform.getOrigin();

	BcReal AmbienceTarget = 0.05f;
	AmbienceVolume_ = ( AmbienceTarget * 0.99f ) + ( AmbienceVolume_ * 0.01f );
	
	Ambience0Emitter_->setGain( AmbienceVolume_ );
	Ambience1Emitter_->setGain( AmbienceVolume_ );

	CameraShakeTimer_ += Tick * 64.0f;
	if( CameraShakeTimer_ > 128.0f )
	{
		CameraShakeTimer_ -= 128.0f;
	}

	BcReal CameraShakeX( BcRandom::Global.interpolatedNoise( CameraShakeTimer_, 128 ) );
	BcReal CameraShakeY( BcRandom::Global.interpolatedNoise( CameraShakeTimer_ + 10.0f, 32 ) );
	BcReal CameraShakeZ( BcRandom::Global.interpolatedNoise( CameraShakeTimer_ + 20.0f, 128 ) );
	BcVec3d CameraShakeVector = BcVec3d( CameraShakeX, CameraShakeY, CameraShakeZ ) * CameraShakeDepth_;
	CameraShakeDepth_ *= 0.95f;

	getParentEntity()->setPosition( BcVec3d( Origin.x(), Origin.y(), Origin.z() ) + CameraShakeVector );

	// Handle movement.
	BcVec3d WalkDirection( 0.0f, 0.0f, 0.0f );
	BcReal TurnSpeed = 2.0f;
	BcReal MoveSpeed = Running_ ? 18.0f : 4.0f;

	BcBool HasChangedView = BcFalse;
	BcBool IsMoving = BcFalse;
	
	if( MoveL_ )
	{
		Rotation_ += Tick * TurnSpeed;
		HasChangedView = BcTrue;
	}
	if( MoveR_ )
	{
		Rotation_ -= Tick * TurnSpeed;
		HasChangedView = BcTrue;
	}

	BcQuat MoveRotation;
	BcVec3d MoveVector( 0.0f, 0.0f, 1.0f );
	MoveRotation.fromEular( Rotation_, 0.0f, 0.0f );

	if( MoveU_ )
	{
		WalkDirection.z( WalkDirection.z() + Tick * TargetMoveSpeed_ );
		HasChangedView = BcTrue;
		IsMoving = BcTrue;
	}
	if( MoveD_ )
	{
		WalkDirection.z( WalkDirection.z() - Tick * TargetMoveSpeed_ );
		HasChangedView = BcTrue;
		IsMoving = BcTrue;
	}

	MoveRotation.rotateVector( WalkDirection );
	getParentEntity()->setRotation( MoveRotation );
	pCharacterController_->setWalkDirection( btVector3( WalkDirection.x(), WalkDirection.y(), WalkDirection.z() ) );

	/* NEILO HACK....broken.
	if( WasOnGround_ == BcFalse && pCharacterController_->onGround() )
	{
		LocalEmitter_->play( LandSound_ );
	}
	*/

	WasOnGround_ = pCharacterController_->onGround();

	// If the view has changed then we need to change draw distance.

	if( IsMoving )
	{
		TargetMoveSpeed_ = TargetMoveSpeed_ * 0.9f + MoveSpeed * 0.1f;
		
		MoveTimer_ += Tick;
		
		BcReal TimerCap = Running_ ? 0.7f : 1.0f;
		ScnSoundRef MoveSound = Running_ ? FootStepFastSound_ : FootStepSlowSound_;
		
		if( MoveTimer_ > TimerCap )
		{
			MoveTimer_ = 0.0f;
			if( WasOnGround_ )
			{
				LocalEmitter_->play( MoveSound );
			}
		}
	}
	else
	{
		MoveTimer_ = 2.0f;
	}

	if( HasChangedView )
	{
		TargetDrawDistance_ = MinDrawDistance_;
		AdjustSpeed_ = 0.003f;
	}
	else
	{
		TargetDrawDistance_ = MaxDrawDistance_;
		AdjustSpeed_ = 0.001f;
	}
	
	ActualDrawDistance_ = ( ( 1.0f - AdjustSpeed_ ) * ActualDrawDistance_ ) + ( AdjustSpeed_ * TargetDrawDistance_ );

	NoiseTextureTimer_ -= Tick;
	if( NoiseTextureTimer_ < 0.0f )
	{
		CurrNoiseTexture_ = ( CurrNoiseTexture_ + BcRandom::Global.randRange( 1, 2 ) ) % 4;
		NoiseTextureTimer_ = BcRandom::Global.randReal() * 0.05f;
	}

	WorldMaterialComponent_->setParameter( InverseDrawDistanceParam_, 1.0f / ActualDrawDistance_ );
	WorldMaterialComponent_->setTexture( NoiseTextureParam_, NoiseTexture_[ CurrNoiseTexture_ ] );

	if( MonsterMaterialComponent_.isValid() )
	{
		MonsterMaterialComponent_->setParameter( InverseDrawDistanceParam_, 1.0f / ActualDrawDistance_ );
		MonsterMaterialComponent_->setTexture( NoiseTextureParam_, NoiseTexture_[ CurrNoiseTexture_ ] );
	}

	// Do a ray cast downwards.
	/*
	BcVec3d Start = getParentEntity()->getTransform().getTranslation();
	BcVec3d End = Start + BcVec3d( 0.0f, -100.0f, 0.0f );
	BcVec3d Intersection;
	if( ScnRigidBodyWorldComponent::StaticGetComponent()->lineCheck( Start, End, Intersection ) )
	{
		BcPrintf( "HIT: %f, %f, %f\n", Intersection.x(), Intersection.y(), Intersection.z() );
	}
	*/

	// Do water drop sounds.
	if( SsCore::pImpl() != NULL )
	{
		BcVec3d Up( 0.0f, 1.0f, 0.0f );
		BcVec3d Forward( 0.0f, 0.0f, 1.0f );
		getParentEntity()->getTransform().getRotation().rotateVector( Up );
		getParentEntity()->getTransform().getRotation().rotateVector( Forward );
		SsCore::pImpl()->setListener( getParentEntity()->getTransform().getTranslation(), Forward, Up );

		// Determine surrounding environment size.
		BcVec3d PlayerPosition = getParentEntity()->getTransform().getTranslation();
		BcBool LinesInsideWorld = BcTrue;
		BcReal MaxSize = 25.0f;
		BcReal DoubleMaxSize = MaxSize * 2.0f;

		LocalEmitter_->setPosition( PlayerPosition );

		Ambience0Emitter_->setPosition( PlayerPosition + BcVec3d( -1.0f, 1.0f, 0.01f ) );
		Ambience1Emitter_->setPosition( PlayerPosition + BcVec3d(  1.0f, 1.0f, 0.01f ) );

		
		// Spherical casts.
		BcAABB Bounds;
		BcReal Delta = BcPIDIV4;
		BcVec3d Point;
		BcReal FurthestPointDistance = 0.0f;
		BcReal RayDistance = 0.0f;
		BcVec3d FurthestPoint;
		for( BcReal Theta = -BcPI; Theta < BcPI; Theta += Delta )
		{
			LinesInsideWorld &= ScnRigidBodyWorldComponent::StaticGetComponent()->lineCheck( PlayerPosition, PlayerPosition + BcVec3d( -BcCos( Theta ), BcSin( Theta ), 0.0f ) * MaxSize, Point );
			Bounds.expandBy( Point );
			RayDistance = ( PlayerPosition - Point ).magnitude();
			if( RayDistance > FurthestPointDistance )
			{
				FurthestPointDistance = RayDistance;
				FurthestPoint = Point;
			}

			LinesInsideWorld &= ScnRigidBodyWorldComponent::StaticGetComponent()->lineCheck( PlayerPosition, PlayerPosition + BcVec3d( 0.0f, -BcCos( Theta ), BcSin( Theta ) ) * MaxSize, Point );
			Bounds.expandBy( Point );
			RayDistance = ( PlayerPosition - Point ).magnitude();
			if( RayDistance > FurthestPointDistance )
			{
				FurthestPointDistance = RayDistance;
				FurthestPoint = Point;
			}

			LinesInsideWorld &= ScnRigidBodyWorldComponent::StaticGetComponent()->lineCheck( PlayerPosition, PlayerPosition + BcVec3d( -BcCos( Theta ), 0.0f, BcSin( Theta ) ) * MaxSize, Point );
			Bounds.expandBy( Point );
			RayDistance = ( PlayerPosition - Point ).magnitude();
			if( RayDistance > FurthestPointDistance )
			{
				FurthestPointDistance = RayDistance;
				FurthestPoint = Point;
			}
		}

		//BcPrintf( "Env size: [%.2f, %.2f, %.2f] (%u)\n", Bounds.width(), Bounds.height(), Bounds.depth(), LinesInsideWorld );

		FurthestPoint.normalise();

		//BcPrintf( "Furthest: [%.2f, %.2f]\n", FurthestPoint.x(), FurthestPoint.z() );

		// Find min (reflection) and max (late reverb) sizes.
		BcReal Volume = Bounds.volume();
		BcReal ReflectionDistance = Bounds.width();
		BcReal LateReverbDistance = Bounds.width();
		ReflectionDistance = BcMin( ReflectionDistance, Bounds.height() );
		ReflectionDistance = BcMin( ReflectionDistance, Bounds.depth() );
		LateReverbDistance = BcMax( LateReverbDistance, Bounds.height() );
		LateReverbDistance = BcMax( LateReverbDistance, Bounds.depth() );

		ReflectionDistance /= DoubleMaxSize;
		LateReverbDistance /= DoubleMaxSize;
		
		//BcPrintf( "Env param: [%.2f, %.2f, %.2f]\n", ReflectionDistance, LateReverbDistance, Volume );

		// Generate parameters based on environment.
		BcReal ReflectionGain = 1.0f;
		BcReal LateReverbGain = 1.0f;

		Environment_.Gain_ = 0.5f;
		Environment_.GainHF_ = 0.4f;
		Environment_.GainLF_ = 1.0f;
		Environment_.Density_ = ReflectionDistance;
		Environment_.ReflectionsGain_ = BcLerp( 0.5f, 3.16f, 1.0f - ReflectionDistance ) * ReflectionGain;
		Environment_.ReflectionsDelay_ = BcLerp( 0.0f, 0.3f, ReflectionDistance );
		Environment_.LateReverbGain_ = BcLerp( 1.0f, 3.0f, 1.0f - LateReverbDistance ) * LateReverbGain;
		Environment_.LateReverbDelay_ = BcLerp( 0.0f, 0.1f, LateReverbDistance );
		Environment_.RoomRolloffFactor_ = 0.75f;

		Environment_.HFReference_ = 0.0f;
		Environment_.LFReference_ = 0.0f;

		SsCore::pImpl()->setEnvironment( Environment_ );
		
		// Handle waterdrop sound.
		WaterDropTimer_ -= Tick;
		if( WaterDropTimer_ < 0.0f )
		{
			BcU32 RetryCount = 32;

			while( RetryCount > 0 )
			{
				// Determine drop position.
				BcReal RandX = BcAbs( BcRandom::Global.randReal() );
				BcReal RandY = BcAbs( BcRandom::Global.randReal() );
				BcReal RandZ = BcAbs( BcRandom::Global.randReal() );
				BcVec3d DropStartPosition;
				DropStartPosition.x( BcLerp( Bounds.min().x(), Bounds.max().x(), RandX ) + FurthestPoint.x() * 5.0f );
				DropStartPosition.y( Bounds.min().y() );
				DropStartPosition.z( BcLerp( Bounds.min().z(), Bounds.max().z(), RandZ ) + FurthestPoint.z() * 5.0f );
				BcVec3d DropEndPosition = DropStartPosition + BcVec3d( 0.0f, -10.0f, 0.0f );
				BcVec3d DropPosition = DropStartPosition;

				if( ScnRigidBodyWorldComponent::StaticGetComponent()->lineCheck( DropStartPosition, DropEndPosition, DropPosition ) )
				{
					// Play sound.
					SsChannel* pChannel = SsCore::pImpl()->play( WaterDropSound_->getSample(), NULL );
					if( pChannel != NULL )
					{
						pChannel->gain( 0.1f );
						pChannel->position( DropPosition );
						pChannel->pitch( BcAbs( BcRandom::Global.randReal() * 0.5f ) + 0.75f );
						pChannel->refDistance( 0.1f );
						pChannel->rolloffFactor( 0.05f );
					}

					RetryCount = 0;
					WaterDropTimer_ = BcAbs( BcRandom::Global.randReal() ) * ( 2.0f / ( ReflectionDistance + LateReverbDistance ) ) * 0.125f;
				}
				else
				{
					--RetryCount;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaPlayerComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );	

	const ScnTransform& Transform = getParentEntity()->getTransform();
	const BcQuat& Rotation = Transform.getRotation();
	const BcVec3d& Translation = Transform.getTranslation();
	btTransform StartTransform( btQuaternion( Rotation.x(), Rotation.y(), Rotation.z(), Rotation.w() ),
								btVector3( Translation.x(), Translation.y(), Translation.z() ) );

	ScnRigidBodyWorldComponentRef RigidBodyWorldComponent = ScnRigidBodyWorldComponent::StaticGetComponent();
	
	btOverlappingPairCache* pOverlappingPairCache = RigidBodyWorldComponent->pBroadphase_->getOverlappingPairCache();
	
	pGhostObject_ = new btPairCachingGhostObject();
	pGhostObject_->setWorldTransform(StartTransform);
	pOverlappingPairCache->setInternalGhostPairCallback( new btGhostPairCallback() );
	btScalar characterHeight=1.75;
	btScalar characterWidth =1.75;
	pShape_ = new btCapsuleShape( characterWidth,characterHeight );
	pGhostObject_->setCollisionShape( pShape_ );
	pGhostObject_->setCollisionFlags( btCollisionObject::CF_CHARACTER_OBJECT );

	btScalar stepHeight = btScalar( 0.35 );
	pCharacterController_ = new btKinematicCharacterController( pGhostObject_, pShape_, stepHeight );

	// Add to world.
	RigidBodyWorldComponent->pDynamicsWorld_->addCollisionObject( pGhostObject_, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter );
	RigidBodyWorldComponent->pDynamicsWorld_->addAction( pCharacterController_ );

	// Bind controls.
	OsEventInputKeyboard::Delegate OnKeyDown = OsEventInputKeyboard::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onKeyDown >( this );
	OsEventInputKeyboard::Delegate OnKeyUp = OsEventInputKeyboard::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onKeyUp >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyDown );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyUp );
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaPlayerComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	ScnRigidBodyWorldComponentRef RigidBodyWorldComponent = ScnRigidBodyWorldComponent::StaticGetComponent();

	// Remove from world.
	RigidBodyWorldComponent->pDynamicsWorld_->removeCollisionObject( pGhostObject_ );
	RigidBodyWorldComponent->pDynamicsWorld_->removeAction( pCharacterController_ );

	// Unbind controls.
	OsCore::pImpl()->unsubscribeAll( this );

	// Stop emitters.
	LocalEmitter_->stopAll();
	Ambience0Emitter_->stopAll();
	Ambience1Emitter_->stopAll();
}

////////////////////////////////////////////////////////////////////////////////
// onKeyDown
eEvtReturn GaPlayerComponent::onKeyDown( EvtID ID, const OsEventInputKeyboard& Event )
{
	switch( Event.KeyCode_ )
	{
	case 'W':
	case 38:
		MoveU_ = BcTrue;
		break;
	case 'S':
	case 40:
		MoveD_ = BcTrue;
		break;
	case 'A':
	case 37:
		MoveL_ = BcTrue;
		break;
	case 'D':
	case 39:
		MoveR_ = BcTrue;
		break;
	case ' ':
		if( pCharacterController_->canJump() )
		{
			LocalEmitter_->stopAll();
			LocalEmitter_->play( JumpSound_ );
			pCharacterController_->jump();
		}
		break;
	case 16:
		Running_ = BcTrue;
		break;
	}
	return evtRET_PASS;	
}

////////////////////////////////////////////////////////////////////////////////
// onKeyUp
eEvtReturn GaPlayerComponent::onKeyUp( EvtID ID, const OsEventInputKeyboard& Event )
{
	switch( Event.KeyCode_ )
	{
	case 'W':
	case 38:
		MoveU_ = BcFalse;
		break;
	case 'S':
	case 40:
		MoveD_ = BcFalse;
		break;
	case 'A':
	case 37:
		MoveL_ = BcFalse;
		break;
	case 'D':
	case 39:
		MoveR_ = BcFalse;
		break;
	case 16:
		Running_ = BcFalse;
		break;
	}

	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// shakeCamera
void GaPlayerComponent::shakeCamera( BcReal Depth )
{
	CameraShakeDepth_ = Depth;
}

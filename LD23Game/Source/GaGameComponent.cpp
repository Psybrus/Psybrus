/**************************************************************************
*
* File:		GaGameComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
* Description:
*		Ball component.
*		
*
*
* 
**************************************************************************/

#include "GaGameComponent.h"

static const BcReal WORLD_SIZE = 32.0f;

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
	
	SpawnTimer_ = 0.0f;

	GameState_ = GS_INIT;

	HeatMapWidth_ = 32;
	HeatMapHeight_ = 32;
	BcU32 Texels = HeatMapWidth_ * HeatMapHeight_;
	pHeatMap_ = new BcReal[ Texels ];
	pHeatMapBuffer_ = new BcReal[ Texels ];
	BcMemZero( pHeatMap_, sizeof( BcReal ) * Texels );
	BcMemZero( pHeatMapBuffer_, sizeof( BcReal ) * Texels );

	for( BcU32 Idx = 0; Idx < Texels; ++Idx )
	{
		pHeatMap_[ Idx ] = 0.5f;
		pHeatMapBuffer_[ Idx ] = 0.5f;
	}

	CsCore::pImpl()->createResource( BcName::INVALID, HeatMapTexture_, HeatMapWidth_, HeatMapHeight_, 1, rsTF_RGBA8 );
}

//////////////////////////////////////////////////////////////////////////
// destroy
void GaGameComponent::destroy()
{
	delete [] pHeatMap_;
	delete [] pHeatMapBuffer_;
	pHeatMap_ = NULL;
	pHeatMapBuffer_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaGameComponent::update( BcReal Tick )
{
	switch( GameState_ )
	{
	case GS_INIT:
		{
			// Max elements to spawn.
			MaxElements_ = 48;

			// Spawn player.
			ScnEntityRef PlayerEntity = ScnCore::pImpl()->createEntity( "default", "PlayerEntity" );
			getParentEntity()->attach( PlayerEntity );

			for( BcU32 Idx = 0; Idx < PlayerEntity->getNoofComponents(); ++Idx )
			{
				ScnComponentRef Component( PlayerEntity->getComponent( Idx ) );
				if( Component->isTypeOf< GaStrongForceComponent >() )
				{
					StrongForce_ = Component;
					break;
				}
			}
			
			// Switch game state.
			GameState_ = GS_UPDATE;
		}
		break;

	case GS_UPDATE:
		{
			// Update.
			updateSimulation( Tick );

			// Update texture.
			updateHeatMapTexture();

			// HUD stuff.
			BcMat4d Projection;
			Projection.orthoProjection( -640.0f, 640.0f, 360.0f, -360.0f, -1.0f, 1.0f );

			Canvas_->clear();
			Canvas_->pushMatrix( Projection );

			//Canvas_->setMaterialComponent( HeatMapMaterial_ );
			//Canvas_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( 700.0f, -700.0f ), 0, RsColour::WHITE, 0 ); 

			// Draw centred.
			BcVec2d Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), "TEST HUD", RsColour::WHITE, BcTrue );
			Font_->draw( Canvas_, BcVec2d( -400.0f, -300.0f ) - Size * 0.5f, "TEST HUD", RsColour::WHITE, BcFalse );

			Canvas_->popMatrix();
		}
		break;

	case GS_EXIT:
		{
		}
		break;
	}
	
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Grab components we need.
	HeatMapModel_ = Parent->getComponentByType< ScnModelComponent >( 0 );
	HeatMapMaterial_ = HeatMapModel_->getMaterialComponent( "heatmap" );
	Canvas_ = Parent->getComponentByType< ScnCanvasComponent >( 0 );
	Font_ = Parent->getComponentByType< ScnFontComponent >( 0 );
	
	// Set heatmap texture.
	BcU32 TextureParam = HeatMapMaterial_->findParameter( "aDiffuseTex" );
	BcU32 ColourParam = HeatMapMaterial_->findParameter( "uColour" );
	HeatMapMaterial_->setTexture( TextureParam, HeatMapTexture_ );
	HeatMapMaterial_->setParameter( ColourParam, RsColour::WHITE );
		
	//
	BcMat4d Scale;
	Scale.scale( BcVec3d( WORLD_SIZE, 1.0f, WORLD_SIZE ) );
	Parent->setMatrix( Scale );

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// spawnElement
void GaGameComponent::spawnElement( const BcVec3d& Position, const BcVec3d& Velocity, const BcName& Type )
{
	ScnEntityRef Entity( ScnCore::pImpl()->createEntity( "default", Type ) );
	BcAssertMsg( Entity.isValid(), "Can't spawn element." );

	// Get element component.
	GaElementComponentRef Element =  Entity->getComponentByType< GaElementComponent >( 0 );
	BcAssertMsg( Element.isValid(), "Element not in entity. Did you use the right template?" );

	// Calculate velocity.
	BcVec3d CalcVelocity = Velocity;
		
	// If velocity is too small just give it a new velocity.
	if( CalcVelocity.magnitude() < Element->MaxSpeed_ )
	{
		CalcVelocity = ( -Position ).normal() * Element->Direction_;

		// Add a little randomisation.
		CalcVelocity += ( BcRandom::Global.randVec3Normal() * 0.3f );
	}

	// Clamp, and TODO: make speeds vary a little.
	CalcVelocity.y( 0.0f );
	CalcVelocity = CalcVelocity.normal() * Element->MaxSpeed_;


	TElement ElementInternal = 
	{
		Type,
		Element->FuseType_,
		Element->ReplaceType_,
		Element->RespawnType_,
		Entity,
		Element,
		BcFalse,
		Position,
		CalcVelocity,
	};

	ElementInternal.Position_.y( 0.0f );

	AddElementList_.push_back( ElementInternal );
}

//////////////////////////////////////////////////////////////////////////
// updateSimulation
void GaGameComponent::updateSimulation( BcReal Tick )
{
	SpawnTimer_ += Tick;

	if( SpawnTimer_ > 0.5f )
	{
		SpawnTimer_ = 0.0f;

		// Spawn new ones if need be.
		if( ElementList_.size() < MaxElements_ )
		{
			for( BcU32 Idx = 0; Idx < 1; ++Idx )
			{
				BcVec3d Position0 = BcRandom::Global.randVec3Normal(); 
				BcVec3d Position1 = BcRandom::Global.randVec3Normal();

				// Clamp y to 0.
				Position0.y( 0.0f );
				Position1.y( 0.0f );
				Position0 = Position0.normal() * WORLD_SIZE;
				Position1 = Position1.normal() * WORLD_SIZE;
				
				// Spawn 1 of each type.
				spawnElement( Position0, BcVec3d( 0.0f, 0.0f, 0.0f ), "In0ElementEntity" );
				spawnElement( Position1, BcVec3d( 0.0f, 0.0f, 0.0f ), "In1ElementEntity" );
			}
		}
	}

	// Do update tick.
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		// Reduce heat when moving through.
		addHeatMapValue( Element.Position_, -0.5f * Tick );

		// Find nearest that isn't us or our type (yeah I know..), and repel based on distance.
		BcU32 OtherIdx = findNearestOfType( Element.Position_, Element.Element_->Radius_, BcName::INVALID, Idx );
		if( OtherIdx != BcErrorCode )
		{
			TElement& OtherElement( ElementList_[ OtherIdx ] );
			
			BcVec3d Direction = Element.Position_ - OtherElement.Position_;
			BcReal DistanceSquared = ( Direction ).magnitudeSquared();
			BcReal TotalRadius = ( Element.Element_->Radius_ + OtherElement.Element_->Radius_ );
			BcReal TotalRadiusSquared = TotalRadius * TotalRadius;
			
			if( DistanceSquared < TotalRadiusSquared )
			{
				// Force ourself away from the other element.
				Element.Velocity_ += Direction.normal() * ( 1.0f - ( DistanceSquared / TotalRadiusSquared ) ) * 1.0f;

				// Clamp velocity.
				Element.Velocity_ = Element.Velocity_.normal() * Element.Element_->MaxSpeed_;
			}
		}

		// Are we inside the radius of the strong force?
		BcReal ForceDistanceSquared = ( Element.Position_ - StrongForce_->Position_ ).magnitudeSquared();
		BcReal ForceRadiusSquared = StrongForce_->Radius_ * StrongForce_->Radius_;

		if( ForceDistanceSquared < ForceRadiusSquared && StrongForce_->IsCharging_ )
		{
			if( Element.FuseType_ != BcName::INVALID && StrongForce_->IsCharging_ )
			{
				// Find nearest one to fuse with and move towards it.
				BcU32 FuseIdx = findNearestOfType( Element.Position_, Element.Element_->Radius_, Element.FuseType_, Idx );
				if( FuseIdx != BcErrorCode )
				{
					TElement& FuseElement( ElementList_[ FuseIdx ] );

					BcVec3d Direction = Element.Position_ - FuseElement.Position_;
					BcReal DistanceSquared = ( Direction ).magnitudeSquared();
					BcReal TotalRadius = ( Element.Element_->Radius_ + FuseElement.Element_->Radius_ );
					BcReal TotalRadiusSquared = TotalRadius * TotalRadius;
			
					if( DistanceSquared < TotalRadiusSquared )
					{
						// Force ourself away from the other element.
						Element.Velocity_ += -Direction.normal() * ( 1.0f - ( DistanceSquared / TotalRadiusSquared ) ) * 2.5f;

						// Clamp velocity.
						Element.Velocity_ = Element.Velocity_.normal() * Element.Element_->MaxSpeed_;
					}
				}

				// Find nearest of ourself and move away.
				BcU32 RepelIdx = findNearestOfType( Element.Position_, Element.Element_->Radius_, Element.Type_, Idx );
				if( RepelIdx != BcErrorCode )
				{
					TElement& RepelElement( ElementList_[ RepelIdx ] );

					BcVec3d Direction = Element.Position_ - RepelElement.Position_;
					BcReal DistanceSquared = ( Direction ).magnitudeSquared();
					BcReal TotalRadius = ( Element.Element_->Radius_ + RepelElement.Element_->Radius_ );
					BcReal TotalRadiusSquared = TotalRadius * TotalRadius;
			
					if( DistanceSquared < TotalRadiusSquared )
					{
						// Force ourself away from the other element.
						Element.Velocity_ += Direction.normal() * ( 1.0f - ( DistanceSquared / TotalRadiusSquared ) ) * 2.5f;

						// Clamp velocity.
						Element.Velocity_ = Element.Velocity_.normal() * Element.Element_->MaxSpeed_;
					}
				}
			}
		}

		// Advance (use heatmap multiplier).
		BcReal HeatMapValue = getHeatMapValue( Element.Position_ ) * 2.0f;
		Element.Position_ += Element.Velocity_ * Tick * HeatMapValue;

		// If it's moving away from the origin and out of bounds remove it.
		if( Element.Position_.magnitude() > WORLD_SIZE &&
			( Element.Position_ + Element.Velocity_ ).magnitudeSquared() > ( Element.Position_ ).magnitudeSquared() )
		{
			// Remove entity.
			RemoveEntityList_.push_back( Element.Entity_ );

			// Spawn new to take it's place on the opposite side.
			if( Element.RespawnType_ != BcName::INVALID )
			{
				BcVec3d Position0 = -Element.Position_;
				spawnElement( Position0, Element.Velocity_, Element.RespawnType_ );
			}
		}

		// Set entity position.
		Element.Entity_->setPosition( Element.Position_ );
	}

	// Mark elements for fusion.
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		// Are we inside the radius of the strong force?
		BcReal ForceDistanceSquared = ( Element.Position_ - StrongForce_->Position_ ).magnitudeSquared();
		BcReal ForceRadiusSquared = StrongForce_->Radius_ * StrongForce_->Radius_;

		if( ForceDistanceSquared < ForceRadiusSquared && StrongForce_->IsActive_ )
		{
			// Find nearest one to fuse with and move towards it.
			if( Element.FuseType_ != BcName::INVALID )
			{
				BcU32 FuseIdx = findNearestOfType( Element.Position_, Element.Element_->Radius_, Element.FuseType_, Idx );
				if( FuseIdx != BcErrorCode )
				{
					TElement& FuseElement( ElementList_[ FuseIdx ] );

					BcReal Distance = ( Element.Position_ - FuseElement.Position_ ).magnitude();
					BcReal TotalRadius = ( Element.Element_->Radius_ + FuseElement.Element_->Radius_ );
			
					if( Distance < ( TotalRadius * 0.25f ) )
					{
						Element.MarkedForFusion_ = BcTrue;
						FuseElement.MarkedForFusion_ = BcTrue;

						BcPrintf( "Marked for fusion: %s & %s\n", (*Element.Type_).c_str(), (*FuseElement.Type_).c_str() );
					}
				}
			}
		}
	}

	// Do the fusion core dance!
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		// It's time to dance!
		if( Element.MarkedForFusion_ && Element.ReplaceType_ != BcName::INVALID )
		{
			// Remove entity.
			RemoveEntityList_.push_back( Element.Entity_ );

			// Spawn new to take it's place on the opposite side.
			spawnElement( Element.Position_, Element.Velocity_, Element.ReplaceType_ );

			addHeatMapValue( Element.Position_, 256.0f );
		}
	}
	
	// Remove entities.
	removeEntities();

	// Add new entities.
	addElements();
}

//////////////////////////////////////////////////////////////////////////
// findNearestOfType
BcU32 GaGameComponent::findNearestOfType( const BcVec3d& Position, BcReal Radius, const BcName& Type, BcU32 Exclude )
{
	BcU32 Nearest = BcErrorCode;
	BcReal NearestDistance = 1e24f;
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		if( Exclude != Idx && ( Type == BcName::INVALID || Element.Type_ == Type ) && Element.MarkedForFusion_ == BcFalse )
		{
			BcReal Distance = ( Position - Element.Position_ ).magnitude();
			BcReal TotalRadius = Radius + Element.Element_->Radius_;

			Distance = BcMax( Distance - TotalRadius, 0.0f );

			if( Distance < NearestDistance )
			{
				Nearest = Idx;
				NearestDistance = Distance;
			}
		}	
	}

	return Nearest;
}

//////////////////////////////////////////////////////////////////////////
// findNearestOfType
BcU32 GaGameComponent::findNearestNotOfType( const BcVec3d& Position, BcReal Radius, const BcName& Type, BcU32 Exclude )
{
	BcU32 Nearest = BcErrorCode;
	BcReal NearestDistance = 1e24f;
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		if( Exclude != Idx && ( Element.Type_ != Type ) && Element.MarkedForFusion_ == BcFalse )
		{
			BcReal Distance = ( Position - Element.Position_ ).magnitude();
			BcReal TotalRadius = Radius + Element.Element_->Radius_;

			Distance = BcMax( Distance - TotalRadius, 0.0f );

			if( Distance < NearestDistance )
			{
				Nearest = Idx;
				NearestDistance = Distance;
			}
		}	
	}

	return Nearest;
}

//////////////////////////////////////////////////////////////////////////
// addElements
void GaGameComponent::addElements()
{
	for( TElementListIterator It( AddElementList_.begin() ); It != AddElementList_.end(); ++It )
	{
		TElement& Element( (*It) );

		ElementList_.push_back( Element );

		// Attach the entity to our parent entity for sequential logic.
		getParentEntity()->attach( Element.Entity_ );
	}

	AddElementList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// removeEntities
void GaGameComponent::removeEntities()
{
	for( TElementListIterator It( ElementList_.begin() ); It != ElementList_.end(); )
	{
		TElement& Element( (*It) );

		if( inRemoveEntityList( Element.Entity_ ) )
		{
			// Detach from ourselves.
			getParentEntity()->detach( Element.Entity_ );

			// Remove from list.
			It = ElementList_.erase( It );
		}
		else
		{
			++It;
		}
	}

	RemoveEntityList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// inRemoveEntityList
BcBool GaGameComponent::inRemoveEntityList( ScnEntityRef Entity )
{
	for( ScnEntityListIterator It( RemoveEntityList_.begin() ); It != RemoveEntityList_.end(); ++It )
	{
		if( (*It) == Entity )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getHeatMapValue
BcReal& GaGameComponent::getHeatMapValue( BcS32 X, BcS32 Y, BcU32 Buffer )
{
	BcU32 Index = BcClamp( X, 0, (BcS32)HeatMapWidth_ - 1 ) + BcClamp( Y, 0, (BcS32)HeatMapHeight_ - 1 ) * HeatMapWidth_;
	return Buffer == 0 ? pHeatMap_[ Index ] : pHeatMapBuffer_[ Index ];
}

//////////////////////////////////////////////////////////////////////////
// updateHeatMapTexture
void GaGameComponent::updateHeatMapTexture()
{
	// Blur X
	for( BcU32 Y = 0; Y < HeatMapHeight_; ++Y )
	{
		for( BcU32 X = 0; X < HeatMapWidth_; ++X )
		{
			BcReal& Out( getHeatMapValue( X, Y, 1 ) );
			BcReal In[] = 
			{
				( getHeatMapValue( X - 2, Y, 0 ) ) * 0.05f,
				( getHeatMapValue( X - 1, Y, 0 ) ) * 0.1f,
				( getHeatMapValue( X, Y, 0 ) ) * 0.7f,
				( getHeatMapValue( X + 1, Y, 0 ) ) * 0.1f,
				( getHeatMapValue( X + 2, Y, 0 ) ) * 0.05f
			};

			Out = ( In[ 0 ] + In[ 1 ] + In[ 2 ] + In[ 3 ] + In[ 4 ] );
		}
	}

	// Blur Y
	for( BcU32 Y = 0; Y < HeatMapHeight_; ++Y )
	{
		for( BcU32 X = 0; X < HeatMapWidth_; ++X )
		{
			BcReal& Out( getHeatMapValue( X, Y, 0 ) );
			BcReal In[] = 
			{
				( getHeatMapValue( X, Y - 2, 1 ) ) * 0.05f,
				( getHeatMapValue( X, Y - 1, 1 ) ) * 0.1f,
				( getHeatMapValue( X, Y, 1 ) ) * 0.7f,
				( getHeatMapValue( X, Y + 1, 1 ) ) * 0.1f,
				( getHeatMapValue( X, Y + 2, 1 ) ) * 0.05f,
			};

			Out = BcMin( ( In[ 0 ] + In[ 1 ] + In[ 2 ] + In[ 3 ] + In[ 4 ] ), 4.0f );
		}
	}

	HeatMapTexture_->lock();

	for( BcU32 Y = 0; Y < HeatMapHeight_; ++Y )
	{
		for( BcU32 X = 0; X < HeatMapWidth_; ++X )
		{
			BcU32 Index = X + Y * HeatMapWidth_;
			BcReal& HeatMapValue( pHeatMap_[ Index ] );
			BcReal ClampedValue = BcClamp( HeatMapValue, 0.0f, 1.0f );
			HeatMapTexture_->setTexel( X, Y, RsColour( ClampedValue, ClampedValue, ClampedValue, 1.0f ) );
		}
	}

	HeatMapTexture_->unlock();
}

//////////////////////////////////////////////////////////////////////////
// addHeatMapValue
void GaGameComponent::addHeatMapValue( const BcVec3d& Position, BcReal Value )
{
	BcVec3d OffsetPosition = Position + BcVec3d( WORLD_SIZE, WORLD_SIZE, WORLD_SIZE );
	BcVec3d PackedPosition( BcVec3d( OffsetPosition / WORLD_SIZE ) * BcVec3d( (BcReal)HeatMapWidth_, 0.0f, (BcReal)HeatMapHeight_ ) * 0.5f );
	BcS32 X = static_cast< BcS32 >( BcClamp( PackedPosition.x(), 0, (BcS32)HeatMapWidth_ - 1 ) );
	BcS32 Y = static_cast< BcS32 >( BcClamp( PackedPosition.z(), 0, (BcS32)HeatMapHeight_ - 1 ) );
	BcReal& HeatMapValue = getHeatMapValue( X, Y );
	HeatMapValue = BcMax( HeatMapValue + Value, 0.0f );
}

//////////////////////////////////////////////////////////////////////////
// getHeatMapValue
BcReal GaGameComponent::getHeatMapValue( const BcVec3d& Position )
{
	BcVec3d OffsetPosition = Position + BcVec3d( WORLD_SIZE, WORLD_SIZE, WORLD_SIZE );
	BcVec3d PackedPosition( BcVec3d( OffsetPosition / WORLD_SIZE ) * BcVec3d( (BcReal)HeatMapWidth_, 0.0f, (BcReal)HeatMapHeight_ ) * 0.5f );
	BcS32 X = static_cast< BcS32 >( BcClamp( PackedPosition.x(), 0, (BcS32)HeatMapWidth_ ) );
	BcS32 Y = static_cast< BcS32 >( BcClamp( PackedPosition.z(), 0, (BcS32)HeatMapHeight_ ) );
	BcReal& HeatMapValue = getHeatMapValue( X, Y );
	return HeatMapValue;	
}

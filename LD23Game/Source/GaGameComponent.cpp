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

#include "GaCameraComponent.h"

BcU32 GaGameComponent::HighScore_ = 0;

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
	
	SpawnTimer_ = 0.0f;

	GameState_ = GS_START_SCREEN;

	HeatMapWidth_ = 32;
	HeatMapHeight_ = 32;
	BcU32 Texels = HeatMapWidth_ * HeatMapHeight_;
	pHeatMap_ = new BcReal[ Texels ];
	pHeatMapBuffer_ = new BcReal[ Texels ];
	BcMemZero( pHeatMap_, sizeof( BcReal ) * Texels );
	BcMemZero( pHeatMapBuffer_, sizeof( BcReal ) * Texels );

	HeatAverage_ = 0.5f;
	for( BcU32 Idx = 0; Idx < Texels; ++Idx )
	{
		pHeatMap_[ Idx ] = HeatAverage_;
		pHeatMapBuffer_[ Idx ] = HeatAverage_;
	}

	HeatMapUpdateAccumulator_ = 0.0f;

	Score_ = 0;
	ScoreMultiplier_ = 1;

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
	// HUD stuff.
	BcMat4d Projection;
	Projection.orthoProjection( -640.0f, 640.0f, 360.0f, -360.0f, -1.0f, 1.0f );

	Canvas_->clear();
	Canvas_->pushMatrix( Projection );

	switch( GameState_ )
	{
	case GS_START_SCREEN:
		{
			// Draw centred.
			BcVec2d Size;
			BcChar Buffer[ 4096 ];
			BcVec2d Position;
			BcMat4d Matrix;

			Matrix.scale( BcVec3d( 2.0f, 2.0f, 1.0f ) );
			Canvas_->pushMatrix( Matrix );	
				Position = BcVec2d( 0.0f, -120.0f );
				BcSPrintf( Buffer, "Fusion Time!" );
				Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::BLACK, BcTrue );
				Font_->draw( Canvas_, Position - Size * 0.5f, Buffer, RsColour::WHITE, BcFalse );
			Canvas_->popMatrix();

			Position = BcVec2d( 0.0f, -180.0f );
			BcSPrintf( Buffer, "By NeiloGD for Ludum Dare 23" );
			Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::BLACK, BcTrue );
			Font_->draw( Canvas_, Position - Size * 0.5f, Buffer, RsColour::WHITE, BcFalse );

			Position = BcVec2d( 0.0f, -150.0f );
			BcSPrintf( Buffer, "Theme: Tiny World" );
			Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::BLACK, BcTrue );
			Font_->draw( Canvas_, Position - Size * 0.5f, Buffer, RsColour::WHITE, BcFalse );


			Position = BcVec2d( 0.0f, 270.0f );
			BcSPrintf( Buffer, "Press any key to continue." );
			Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::BLACK, BcTrue );
			Font_->draw( Canvas_, Position - Size * 0.5f, Buffer, RsColour::WHITE, BcFalse );
		}
		break;

	case GS_INSTRUCTIONS:
		{
			// Draw centred.
			BcVec2d Size;
			BcChar Buffer[ 4096 ];
			BcVec2d Position;
			BcMat4d Matrix;
			
			Matrix.scale( BcVec3d( 1.5f, 1.5f, 1.0f ) );
			Canvas_->pushMatrix( Matrix );	
				Position = BcVec2d( 0.0f, -180.0f );
				BcSPrintf( Buffer, "Instructions" );
				Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::BLACK, BcTrue );
				Font_->draw( Canvas_, Position - Size * 0.5f, Buffer, RsColour::WHITE, BcFalse );
			Canvas_->popMatrix();

			const BcChar* Lines[] = 
			{
				"*** Overview ***",
				"You are strong force! You must fuse",
				"red and blue to keep the sun burning!",
				" - ",
				"*** Click LMB to start charging ***",
				"Nearby atoms are drawn towards you when you charge.",
				" - ",
				"*** Release LMB to activate ***",
				"If you have a red and blue atom very close togheter",
				"when you release, they will fuse and give energy to",
				"the sun. Be careful, charging sucks energy so time",
				"it as well as you can!",
				" - ",
				"*** Scoring ***",
				"Your multiplier will keep going up as",
				"long as you don't miss a fusion.",
				" - ",
				"Press any key to begin!"
			};

			BcU32 NoofLines = sizeof( Lines ) / sizeof( Lines[ 0 ] );

			Position = BcVec2d( 0.0f, -170.0f );

			for( BcU32 Idx = 0; Idx < NoofLines; ++Idx )
			{
				Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Lines[ Idx ], RsColour::BLACK, BcTrue );
				Font_->draw( Canvas_, Position - Size * 0.5f, Lines[ Idx ], RsColour::WHITE, BcFalse );
				Position.y( Position.y() + Size.y() );
			}
		}
		break;

	case GS_INIT:
		{
			GaCameraComponent::pImpl()->startGame();

			// Max elements to spawn.
			MaxElements_ = 64;
			
			// Spawn player.
			ScnEntityRef PlayerEntity = ScnCore::pImpl()->createEntity( "game", "PlayerEntity" );
			getParentEntity()->attach( PlayerEntity );

			StrongForce_ = PlayerEntity->getComponentByType< GaStrongForceComponent >( 0 );
			
			// Switch game state.
			GameState_ = GS_UPDATE;
		}
		break;

	case GS_UPDATE:
		{
			// Update.
			updateSimulation( Tick );

			// Update texture.
			updateHeatMapTexture( Tick );
			
			// Do particle plasma cloud.
			particlesPlasmaCloud();

			// Do the game over.
			if( HeatAverage_ < 0.1f )
			{
				GameState_ = GS_GAME_OVER;
				HighScore_ = BcMax( HighScore_, Score_ );

				SoundEmitter_->setGain( 1.0f );
				SoundEmitter_->play( "default", "lost" );
				SoundEmitter_->play( "default", "forceexplode" );

				GaCameraComponent::pImpl()->startMenu();

				// Remove player.
				getParentEntity()->detach( StrongForce_->getParentEntity() );
			}
		}
		break;

	case GS_GAME_OVER:
		{
			// Draw centred.
			BcVec2d Size;
			BcChar Buffer[ 4096 ];
			BcVec2d Position;
			BcMat4d Matrix;
			
			Matrix.scale( BcVec3d( 1.5f, 1.5f, 1.0f ) );
			Canvas_->pushMatrix( Matrix );	
				Position = BcVec2d( 0.0f, -180.0f );
				BcSPrintf( Buffer, "GAME OVER" );
				Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::BLACK, BcTrue );
				Font_->draw( Canvas_, Position - Size * 0.5f, Buffer, RsColour::WHITE, BcFalse );
			Canvas_->popMatrix();


			const BcChar* Lines[] = 
			{
				( HighScore_ == Score_ ) ? "You got a high score!" : "You didn't get a high score.",
				" - ",
				" - ",
				" - ",
				"Press any key to try again!"
			};

			BcU32 NoofLines = sizeof( Lines ) / sizeof( Lines[ 0 ] );

			Position = BcVec2d( 0.0f, -140.0f );

			for( BcU32 Idx = 0; Idx < NoofLines; ++Idx )
			{
				Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Lines[ Idx ], RsColour::BLACK, BcTrue );
				Font_->draw( Canvas_, Position - Size * 0.5f, Lines[ Idx ], RsColour::WHITE, BcFalse );
				Position.y( Position.y() + Size.y() );
			}
		}
		break;

	case GS_EXIT:
		{
		}
		break;
	}

	// Draw centred.
	BcVec2d Size;
	BcChar Buffer[ 4096 ];
	BcVec2d Position = BcVec2d( -450.0f, -270.0f );
	BcSPrintf( Buffer, "Best: %u\nScore: %u\nMultiplier: %u\n", HighScore_, Score_, ScoreMultiplier_ );
	Size = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Buffer, RsColour::BLACK, BcTrue );
	//Canvas_->setMaterialComponent( TextBgMaterial_ );
	//Canvas_->drawSpriteCentered( Position, Size * 1.75f, 0, RsColour::BLACK, 0 );
	Font_->draw( Canvas_, Position - Size * 0.5f, Buffer, RsColour::WHITE, BcFalse );

	static BcReal Ticer = 0.0f;
	Ticer += Tick;
	
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Grab components we need.
	TextBgMaterial_ = Parent->getComponentByType< ScnMaterialComponent >( 0 );
	Canvas_ = Parent->getComponentByType< ScnCanvasComponent >( 0 );
	Font_ = Parent->getComponentByType< ScnFontComponent >( 0 );
	ParticleSystem_ = Parent->getComponentByType< ScnParticleSystemComponent >( 0 );

	// Font
	{
		ScnMaterialComponentRef Material = Font_->getMaterialComponent();
		BcU32 Param = Material->findParameter( "aAlphaTestStep" );
		Material->setParameter( Param, BcVec2d( 0.3f, 0.9f ) );
	}
	
	// Set heatmap texture in model.
	HeatMapModel_ = Parent->getComponentByType< ScnModelComponent >( 0 );
	if( HeatMapModel_.isValid() )
	{
		BcMat4d ScaleMatrix;
		ScaleMatrix.scale( BcVec3d( WORLD_SIZE, WORLD_SIZE, WORLD_SIZE ) );
		HeatMapModel_->setTransform( 0, ScaleMatrix );
		HeatMapMaterial_ = HeatMapModel_->getMaterialComponent( "heatmap" );
		if( HeatMapMaterial_.isValid() )
		{
			BcU32 TextureParam = HeatMapMaterial_->findParameter( "aDiffuseTex" );
			BcU32 ColourParam = HeatMapMaterial_->findParameter( "uColour" );
			HeatMapMaterial_->setTexture( TextureParam, HeatMapTexture_ );
			HeatMapMaterial_->setParameter( ColourParam, RsColour::WHITE );
		}
	}

	// Set heatmap texture in particle system.
	if( ParticleSystem_.isValid() )
	{
		BcReal ScaleFactor = 0.5f / WORLD_SIZE;
		BcMat4d HeatMapMatrix;
		HeatMapMatrix.row0( BcVec4d( ScaleFactor,	0.0f,			0.0f,			0.0f ) );
		HeatMapMatrix.row1( BcVec4d( 0.0f,			0.0f,			-ScaleFactor,	0.0f ) );
		HeatMapMatrix.row2( BcVec4d( 0.0f,			ScaleFactor,	0.0f,			0.0f ) );
		HeatMapMatrix.row3( BcVec4d( 0.5f,			0.5f,			0.5f,			1.0f ) );
		ScnMaterialComponentRef MaterialRef = ParticleSystem_->getMaterialComponent();
		BcU32 HeatMapTextureParam = MaterialRef->findParameter( "aHeatMapTex" );
		BcU32 HeatMapMatrixParam = MaterialRef->findParameter( "uHeatMapMatrix" );
		MaterialRef->setTexture( HeatMapTextureParam, HeatMapTexture_ );
		MaterialRef->setParameter( HeatMapMatrixParam, HeatMapMatrix );
	}

	SoundEmitter_ = Parent->getComponentByType< ScnSoundEmitterComponent >( 0 );

	OsEventInputKeyboard::Delegate OnKeyboardEvent = OsEventInputKeyboard::Delegate::bind< GaGameComponent, &GaGameComponent::onKeyboardEvent >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyboardEvent );
	
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// spawnElement
void GaGameComponent::spawnElement( const BcVec3d& Position, const BcVec3d& Velocity, const BcName& Type )
{
	ScnEntityRef Entity( ScnCore::pImpl()->createEntity( "game", Type ) );
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
		CalcVelocity += ( BcRandom::Global.randVec3Normal() * 0.1f );
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

	if( SpawnTimer_ > 2.0f )
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

				// Pick a random position to move towards.
				BcVec3d Target = BcRandom::Global.randVec3() * ( WORLD_SIZE * 0.5f );

				BcVec3d Velocity0 = ( Target - Position0 ).normal() * 100.0f;
				BcVec3d Velocity1 = ( Target - Position1 ).normal() * 100.0f;
				
				// Spawn 1 of each type.
				spawnElement( Position0, Velocity0, "In0ElementEntity" );
				spawnElement( Position1, Velocity1, "In1ElementEntity" );
			}
		}
	}

	// Do update tick.
	for( BcU32 Idx = 0; Idx < ElementList_.size(); ++Idx )
	{
		TElement& Element( ElementList_[ Idx ] );

		// Reduce heat when moving through.
		addHeatMapValue( Element.Position_, -1.5f * Tick );

		// Find nearest that isn't us or our type (yeah I know..), and repel based on distance.
		BcU32 OtherIdx = findNearestOfType( Element.Position_, Element.Element_->Radius_, BcName::INVALID, Idx );
		if( OtherIdx != BcErrorCode )
		{
			TElement& OtherElement( ElementList_[ OtherIdx ] );
			
			BcVec3d Direction = Element.Position_ - OtherElement.Position_;
			BcReal DistanceSquared = ( Direction ).magnitudeSquared();
			BcReal TotalRadius = ( Element.Element_->Radius_ + OtherElement.Element_->Radius_ );
			BcReal TotalRadiusSquared = TotalRadius * TotalRadius;
			BcReal TwiceTotalRadius = ( Element.Element_->Radius_ + OtherElement.Element_->Radius_ ) * 2.0f;
			BcReal TwiceTotalRadiusSquared = TwiceTotalRadius * TwiceTotalRadius;
			
			if( DistanceSquared < TotalRadiusSquared )
			{
				// Force ourself away from the other element.
				Element.Velocity_ += Direction.normal() * ( 1.0f - ( DistanceSquared / TotalRadiusSquared ) ) * 1.0f;

				// Clamp velocity.
				Element.Velocity_ = Element.Velocity_.normal() * Element.Element_->MaxSpeed_;
			}

			// Signal some are ready to fuse.
			if( Element.FuseType_ == OtherElement.Type_ )
			{
				if( DistanceSquared < TwiceTotalRadius )
				{
					BcVec3d AvgPosition( ( Element.Position_ + OtherElement.Position_ ) * 0.5f );
					particlesCollision( AvgPosition );

					static BcName Collision[] =
					{
						BcName( "collide0" ),
						BcName( "collide1" ),
						BcName( "collide2" ),
						BcName( "collide3" ),
					};


					Element.Element_->playSound( Collision[ BcRandom::Global.randRange( 0, 3 ) ] );
				}
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
					BcReal TotalRadius = ( Element.Element_->Radius_ + FuseElement.Element_->Radius_ ) * 2.0f;
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
		BcReal HeatMapValue = getHeatMapValue( Element.Position_ ) * 8.0f;
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
			
					if( Distance < ( TotalRadius * 0.5f ) )
					{
						Element.MarkedForFusion_ = BcTrue;
						FuseElement.MarkedForFusion_ = BcTrue;

						BcPrintf( "Marked for fusion: %s & %s\n", (*Element.Type_).c_str(), (*FuseElement.Type_).c_str() );
					}
				}
			}
		}
	}

	BcU32 NoofFused = 0;

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

			// Radius makes explody smaller.
			BcReal HeatMultiplier = 16.0f - StrongForce_->Radius_;
			BcReal ColourMultiplier = HeatMultiplier / 16.0f;
			particlesFusionExplode( Element.Position_, Element.Velocity_, Element.Element_->Colour_ * ColourMultiplier );
			addHeatMapValue( Element.Position_, 768.0f * HeatMultiplier );

			static BcName Fuse( "fuse" );
			StrongForce_->playSound( Fuse );

			Score_ += 5  * ScoreMultiplier_;

			++NoofFused;
		}
	}

	if( StrongForce_->IsCharging_ )
	{
		particlesFusionCharge( StrongForce_->Position_, StrongForce_->Radius_ );

		// Reduce heat when charging.
		addHeatMapValue( StrongForce_->Position_, -StrongForce_->Radius_ * 1024.0f );
	}
	
	if( StrongForce_->IsActive_ )
	{
		particlesFusionActivate( StrongForce_->Position_ );

		if( NoofFused > 0 )
		{
			ScoreMultiplier_ += NoofFused;
		}
		else
		{
			if( ScoreMultiplier_ > 1 )
			{
				static BcName LostMultiplier( "lostmultiplier" );
				StrongForce_->playSound( LostMultiplier );
			}
			ScoreMultiplier_ = 1;
		}
	}

	
	// Remove entities.
	removeEntities();

	// Add new entities.
	addElements();

	// HACK. SHOULD BE EVENT DRIVEN.
	if( StrongForce_->IsActive_ )
	{
		StrongForce_->IsActive_ = BcFalse;
	}
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
void GaGameComponent::updateHeatMapTexture( BcReal Tick )
{
	HeatMapUpdateAccumulator_ += Tick;

	while( HeatMapUpdateAccumulator_ > 0.0f )
	{
		// Calculate falloff for blur.
		BcVec3d Falloff( 2.0f, 0.1f, 0.5f );
		BcReal Total = Falloff.x() + ( Falloff.y() + Falloff.z() ) * 2.0f;
		Falloff /= Total;
	
	
		// Blur X
		for( BcU32 Y = 0; Y < HeatMapHeight_; ++Y )
		{
			for( BcU32 X = 0; X < HeatMapWidth_; ++X )
			{
				BcReal& Out( getHeatMapValue( X, Y, 1 ) );
				BcReal In[] = 
				{
					( getHeatMapValue( X - 2, Y, 0 ) ) * Falloff.z(),
					( getHeatMapValue( X - 1, Y, 0 ) ) * Falloff.y(),
					( getHeatMapValue( X, Y, 0 ) ) * Falloff.x(),
					( getHeatMapValue( X + 1, Y, 0 ) ) * Falloff.y(),
					( getHeatMapValue( X + 2, Y, 0 ) ) * Falloff.z()
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
					( getHeatMapValue( X, Y - 2, 1 ) ) * Falloff.z(),
					( getHeatMapValue( X, Y - 1, 1 ) ) * Falloff.y(),
					( getHeatMapValue( X, Y, 1 ) ) * Falloff.x(),
					( getHeatMapValue( X, Y + 1, 1 ) ) * Falloff.y(),
					( getHeatMapValue( X, Y + 2, 1 ) ) * Falloff.z(),
				};

				Out = BcClamp( ( In[ 0 ] + In[ 1 ] + In[ 2 ] + In[ 3 ] + In[ 4 ] ), 0.0f, 4.0f );
			}
		}

		// Decrement accumulator.
		HeatMapUpdateAccumulator_ -= 1.0f / 60.0f;
	}

	// Write out to texture & calculate average heat.
	HeatAverage_ = 0.0f;
	HeatMapTexture_->lock();

	for( BcU32 Y = 0; Y < HeatMapHeight_; ++Y )
	{
		for( BcU32 X = 0; X < HeatMapWidth_; ++X )
		{
			BcU32 Index = X + Y * HeatMapWidth_;
			BcReal& HeatMapValue( pHeatMap_[ Index ] );
			HeatAverage_ += HeatMapValue;
			BcReal ClampedValue = BcClamp( HeatMapValue, 0.0f, 1.0f );
			ClampedValue = ClampedValue * ClampedValue;
			HeatMapTexture_->setTexel( X, Y, RsColour( ClampedValue, ClampedValue, ClampedValue, 1.0f ) );
		}
	}

	HeatAverage_ /= BcReal( HeatMapWidth_ * HeatMapHeight_ );

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

//////////////////////////////////////////////////////////////////////////
// particlesPlasmaCloud
void GaGameComponent::particlesPlasmaCloud()
{
	for( BcU32 Idx = 0; Idx < 4; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ParticleSystem_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = BcRandom::Global.randVec3() * BcVec3d( 32.0f, 4.0f, 32.0f );
			pParticle->Velocity_ = BcRandom::Global.randVec3Normal() * BcVec3d( 1.0f, 1.0f, 1.0f );
			pParticle->Acceleration_ = -pParticle->Velocity_ * 1.0f;
			pParticle->Rotation_ = ( BcRandom::Global.randReal() * BcPIMUL2 );
			pParticle->RotationMultiplier_ = ( BcRandom::Global.randReal() * 0.5f );
			pParticle->MinScale_ = BcVec2d( 16.0f, 16.0f );
			pParticle->MaxScale_ = BcVec2d( 16.0f, 16.0f );
			pParticle->MinColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
			pParticle->MaxColour_ = RsColour( 0.5f, 0.25f, 0.0f, 0.0f );
			pParticle->TextureIndex_ = (BcU32)BcRandom::Global.randRange( 0, 7 );
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = BcAbs( BcRandom::Global.randReal() * 1.5f ) + 1.0f;
			pParticle->Alive_ = BcTrue;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// particlesFusionExplode
void GaGameComponent::particlesFusionExplode( const BcVec3d& Position, const BcVec3d& Direction, const RsColour& Colour )
{
	for( BcU32 Idx = 0; Idx < 128; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ParticleSystem_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = Position + BcRandom::Global.randVec3() * 1.0f;
			pParticle->Velocity_ = BcRandom::Global.randVec3Normal() * 64.0f;
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			pParticle->Rotation_ = ( BcRandom::Global.randReal() * BcPIMUL2 );
			pParticle->RotationMultiplier_ = ( BcRandom::Global.randReal() * 4.5f );
			pParticle->MinScale_ = BcVec2d( 1.0f, 1.0f );
			pParticle->MaxScale_ = BcVec2d( 2.0f, 2.0f );
			pParticle->MinColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
			pParticle->MaxColour_ = RsColour( 1.0f, 1.0f, 1.0f, 0.0f ) * Colour;
			pParticle->TextureIndex_ = (BcU32)BcRandom::Global.randRange( 8, 9 );
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = BcAbs( BcRandom::Global.randReal() * 0.2f ) + 0.2f;
			pParticle->Alive_ = BcTrue;
		}
	}

	for( BcU32 Idx = 0; Idx < 32; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ParticleSystem_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = Position + BcRandom::Global.randVec3() * 0.25f;
			pParticle->Velocity_ = BcRandom::Global.randVec3Normal() * 16.0f;
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			pParticle->Rotation_ = ( BcRandom::Global.randReal() * BcPIMUL2 );
			pParticle->RotationMultiplier_ = ( BcRandom::Global.randReal() * 4.5f );
			pParticle->MinScale_ = BcVec2d( 2.0f, 2.0f );
			pParticle->MaxScale_ = BcVec2d( 4.0f, 4.0f );
			pParticle->MinColour_ = RsColour( 1.0f, 1.0f, 1.0f, 0.0f );
			pParticle->MaxColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
			pParticle->TextureIndex_ = (BcU32)BcRandom::Global.randRange( 10, 11 );
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = BcAbs( BcRandom::Global.randReal() * 0.5f ) + 0.1f;
			pParticle->Alive_ = BcTrue;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// particlesCollision
void GaGameComponent::particlesCollision( const BcVec3d& Position )
{
	for( BcU32 Idx = 0; Idx < 4; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ParticleSystem_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = Position + BcRandom::Global.randVec3() * 1.0f;
			pParticle->Velocity_ = BcRandom::Global.randVec3Normal() * 24.0f;
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			pParticle->Rotation_ = ( BcRandom::Global.randReal() * BcPIMUL2 );
			pParticle->RotationMultiplier_ = ( BcRandom::Global.randReal() * 4.5f );
			pParticle->MinScale_ = BcVec2d( 1.0f, 1.0f ) * BcRandom::Global.randReal();
			pParticle->MaxScale_ = pParticle->MinScale_ * 2.0f;
			pParticle->MinColour_ = RsColour( 1.0f, 0.0f, 0.0f, 1.0f );
			pParticle->MaxColour_ = RsColour( 1.0f, 1.0f, 0.0f, 0.0f );
			pParticle->TextureIndex_ = (BcU32)BcRandom::Global.randRange( 8, 9 );
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = BcAbs( BcRandom::Global.randReal() * 0.1f ) + 0.12f;
			pParticle->Alive_ = BcTrue;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// particlesFusionCharge
void GaGameComponent::particlesFusionCharge( const BcVec3d& Position, BcReal Radius )
{
	BcU32 Count = BcU32( ( BcPIMUL4 * Radius * Radius ) * 0.08f );
	for( BcU32 Idx = 0; Idx < Count; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ParticleSystem_->allocParticle( pParticle ) )
		{
			BcVec3d RelPosition = BcRandom::Global.randVec3Normal();
			pParticle->Position_ = Position + RelPosition * Radius;
			pParticle->Velocity_ = -RelPosition * 16.0f;
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			pParticle->Rotation_ = ( BcRandom::Global.randReal() * BcPIMUL2 );
			pParticle->RotationMultiplier_ = ( BcRandom::Global.randReal() * 4.5f );
			pParticle->MinScale_ = BcVec2d( 0.7f, 0.7f );
			pParticle->MaxScale_ = BcVec2d( 2.0f, 2.0f );
			pParticle->MinColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
			pParticle->MaxColour_ = RsColour( 1.0f, 0.1f, 1.0f, 0.0f );
			pParticle->TextureIndex_ = (BcU32)BcRandom::Global.randRange( 8, 11 );
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = BcAbs( BcRandom::Global.randReal() * 0.09f ) + 0.04f;
			pParticle->Alive_ = BcTrue;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// particlesFusionActivate
void GaGameComponent::particlesFusionActivate( const BcVec3d& Position )
{
	for( BcU32 Idx = 0; Idx < 1; ++Idx )
	{
		ScnParticle* pParticle = NULL;
		if( ParticleSystem_->allocParticle( pParticle ) )
		{
			pParticle->Position_ = Position + BcRandom::Global.randVec3();
			pParticle->Velocity_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			pParticle->Acceleration_ = BcVec3d( 0.0f, 0.0f, 0.0f );
			pParticle->Rotation_ = ( BcRandom::Global.randReal() * BcPIMUL2 );
			pParticle->RotationMultiplier_ = ( BcRandom::Global.randReal() * 4.5f );
			pParticle->MinScale_ = BcVec2d( 0.5f, 0.5f );
			pParticle->MaxScale_ = BcVec2d( 64.0f, 64.0f );
			pParticle->MinColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
			pParticle->MaxColour_ = RsColour( 0.1f, 0.1f, 0.1f, 0.0f );
			pParticle->TextureIndex_ = 12;
			pParticle->CurrentTime_ = 0.0f;
			pParticle->MaxTime_ = BcAbs( BcRandom::Global.randReal() * 0.1f ) + 0.12f;
			pParticle->Alive_ = BcTrue;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onKeyboardEvent
eEvtReturn GaGameComponent::onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	switch( GameState_ )
	{
	case GS_START_SCREEN:
		{
			GameState_ = GS_INSTRUCTIONS;
		}
		break;

	case GS_INSTRUCTIONS:
		{
			GameState_ = GS_INIT;
		}
		break;

	case GS_GAME_OVER:
		{
			ScnEntity* pEntity = getParentEntity();
			ScnCore::pImpl()->removeEntity( pEntity );
			ScnCore::pImpl()->addEntity( ScnCore::pImpl()->createEntity( "game", "GameEntity" ) );
		}

	default:
		break;
	}

	return evtRET_PASS;
}
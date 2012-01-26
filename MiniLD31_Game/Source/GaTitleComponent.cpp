/**************************************************************************
*
* File:		GaTitleComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World.
*		
*
*
* 
**************************************************************************/

#include "GaTitleComponent.h"

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaTitleComponent )

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaTitleComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaTitleComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaTitleComponent::initialise( ScnMaterialRef Material, BcReal Time, BcReal BestTime ) 
{
	Fade_ = 0.0f;
	FadeStage_ = FS_IN;
	Time_ = Time;
	BestTime_ = BestTime;

	CsCore::pImpl()->createResource( BcName::INVALID, FontComponent_, ScnFont::Default, Material );
}

////////////////////////////////////////////////////////////////////////////////
// update 
//virtual
void GaTitleComponent::update( BcReal Tick )
{
	static BcReal Timer = 0.0f;
	Timer += Tick * 0.75f;
	BcReal Breathing = ( ( BcSin( Timer * 2.7f ) + 1.0f ) * 0.05f ) + 0.1f;
	BcReal L = 0.0f;
	BcReal H = 0.0f;
	Fade_ += Tick * 0.25f;
	switch( FadeStage_ )
	{
	case FS_IN:
		{
			L = ( ( 1.0f - BcSmoothStep( Fade_ ) ) ) * 0.75f;
			H = L + Breathing;
			if( Fade_ > 1.0f )
			{
				Fade_ = 0.0f;
				FadeStage_ = FS_WAIT;
			}
		}
		break;
		
	case FS_WAIT:
		{
			L = 0.0f;
			H = L + Breathing;
			if( Fade_ > 1.0f )
			{
				Fade_ = 0.0f;
				FadeStage_ = FS_OUT;
			}
		}
		break;
		
	case FS_OUT:
		{
			L = 0.0f + ( BcSmoothStep( Fade_ ) ) * 0.75f;
			H = L + Breathing;
			if( Fade_ > 1.0f )
			{
				Fade_ = 0.0f;
				GaTopState::pImpl()->startGame();
				return;
			}
		}
		break;
	}


	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	BcMat4d Projection; 
	BcReal Aspect = (BcReal)pClient->getWidth() / (BcReal)pClient->getHeight();
	BcReal DesiredHeight = 240.0f;
	BcReal DesiredWidth = Aspect * DesiredHeight;
	Projection.orthoProjection( -DesiredWidth, DesiredWidth, DesiredHeight, -DesiredHeight, -1.0f, 1.0f );

	CanvasComponent_->clear();

	ScnMaterialComponentRef MaterialComponent = CanvasComponent_->getMaterialComponent();
	BcU32 AlphaTestStep;
	AlphaTestStep = FontComponent_->getMaterialComponent()->findParameter( "aAlphaTestStep" );
	BcReal AlphaStepVal = L * 0.01f;
	FontComponent_->getMaterialComponent()->setParameter( AlphaTestStep, BcVec2d( 0.50f, 0.55f ) );
	AlphaTestStep = MaterialComponent->findParameter( "aAlphaTestStep" );
	MaterialComponent->setParameter( AlphaTestStep, BcVec2d( L, H ) );
	
	CanvasComponent_->pushMatrix( Projection );

	CanvasComponent_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( 640.0f, 480.0f ), 0, RsColour::RED, 0 );

	BcChar Buffer[ 512 ];

	{
		BcMat4d Transform;
		Transform.scale( BcVec3d( 0.3f, 0.3f, 1.0f ) );
		Transform.translation( BcVec3d( -320.0f, 210.0f, 0.0f ) );
		CanvasComponent_->pushMatrix( Transform );
		BcSPrintf( Buffer, "BEST TIME: %.1f seconds", BestTime_ );
		FontComponent_->draw( CanvasComponent_, Buffer, RsColour::RED * RsColour( 1.0f, 1.0f, 1.0f, ( 0.75f - L ) + Breathing ) );
		CanvasComponent_->popMatrix();
	}

	{
		BcMat4d Transform;
		Transform.scale( BcVec3d( 0.3f, 0.3f, 1.0f ) );
		Transform.translation( BcVec3d( -320.0f, 230.0f, 0.0f ) );
		CanvasComponent_->pushMatrix( Transform );
		BcSPrintf( Buffer, "PREV TIME: %.1f seconds", Time_ );
		FontComponent_->draw( CanvasComponent_, Buffer, RsColour::RED * RsColour( 1.0f, 1.0f, 1.0f, ( 0.75f - L ) + Breathing ) );
		CanvasComponent_->popMatrix();
	}

	{
		BcMat4d Transform;
		Transform.scale( BcVec3d( 0.3f, 0.3f, 1.0f ) );
		Transform.translation( BcVec3d( 100.0f, 210.0f, 0.0f ) );
		CanvasComponent_->pushMatrix( Transform );
		BcSPrintf( Buffer, "A MINILD 31 GAME" );
		FontComponent_->draw( CanvasComponent_, Buffer, RsColour::RED * RsColour( 1.0f, 1.0f, 1.0f, ( 0.75f - L ) + Breathing ) );
		CanvasComponent_->popMatrix();
	}

	{
		BcMat4d Transform;
		Transform.scale( BcVec3d( 0.3f, 0.3f, 1.0f ) );
		Transform.translation( BcVec3d( 100.0f, 230.0f, 0.0f ) );
		CanvasComponent_->pushMatrix( Transform );
		BcSPrintf( Buffer, "- BY NEILOGD" );
		FontComponent_->draw( CanvasComponent_, Buffer, RsColour::RED * RsColour( 1.0f, 1.0f, 1.0f, ( 0.75f - L ) + Breathing ) );
		CanvasComponent_->popMatrix();
	}

	CanvasComponent_->popMatrix();
	CanvasComponent_->popMatrix();
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaTitleComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );	

	for( BcU32 Idx = 0; Idx < getParentEntity()->getNoofComponents(); ++Idx )
	{
		ScnCanvasComponentRef CanvasComponent( getParentEntity()->getComponent( Idx ) );
		if( CanvasComponent.isValid() )
		{
			CanvasComponent_ = CanvasComponent;
			break;
		}
	}

	Parent->attach( FontComponent_ );
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaTitleComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Parent->detach( FontComponent_ );
}

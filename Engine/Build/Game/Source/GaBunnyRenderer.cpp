/**************************************************************************
*
* File:		GaBunnyRenderer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderer for bunny ^_^
*		
*
*
* 
**************************************************************************/

#include "GaBunnyRenderer.h"

#include "GaTopState.h"



////////////////////////////////////////////////////////////////////////////////
// Ctor
GaBunnyRenderer::GaBunnyRenderer()
{
	ScnMaterialRef Material;
	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_BUNNY, Material );
	Material->createInstance( "bunnymaterialinstance", MaterialInstance_, BcErrorCode );

	// Feet offsets.
	Layers_[ LAYER_FOOT_REAR_0 ].TimeTicker_ = 0.5f;
	Layers_[ LAYER_FOOT_FRONT_0 ].TimeTicker_ = 0.0f;
	Layers_[ LAYER_FOOT_FRONT_1 ].TimeTicker_ = 0.1f;

	// No body anim.
	Layers_[ LAYER_BODY ].Multiplier_ = BcVec2d( 0.0f, 0.0f );

	// Ear bounce.
	Layers_[ LAYER_EARS ].Multiplier_ = BcVec2d( 2.0f, 4.0f );

	// Head.
	Layers_[ LAYER_HEAD ].Multiplier_ = BcVec2d( 2.0f, 2.0f );
	
	// Nose.
	Layers_[ LAYER_NOSE ].Multiplier_ = BcVec2d( 2.0f, 3.0f );

	// Eyes.
	Layers_[ LAYER_EYES ].Multiplier_ = BcVec2d( 2.0f, 2.0f );

	// Tail.
	Layers_[ LAYER_TAIL ].Multiplier_ = BcVec2d( 2.0f, 2.0f );
	Layers_[ LAYER_TAIL ].TimeSpeed_ = 4.4f;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaBunnyRenderer::~GaBunnyRenderer()
{
}

////////////////////////////////////////////////////////////////////////////////
// update
BcBool GaBunnyRenderer::isReady()
{
	return MaterialInstance_.isReady();
}

////////////////////////////////////////////////////////////////////////////////
// update
void GaBunnyRenderer::update( BcReal Tick )
{
	for( BcU32 Idx = 0; Idx < LAYER_MAX; ++Idx )
	{
		TLayer& Layer = Layers_[ Idx ];
		Layer.TimeTicker_ += Tick * Layer.TimeSpeed_;
		
		if( Layer.TimeTicker_ > 1.0f )
		{
			Layer.TimeTicker_ -= 1.0f;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaBunnyRenderer::render( ScnCanvasRef Canvas, const BcVec2d& Position )
{
	BcMat4d PositionMatrix;

	PositionMatrix.scale( BcVec3d( 1.0f, -1.0f, 1.0f ) );
	PositionMatrix.translation( Position );

	Canvas->pushMatrix( PositionMatrix );

	for( BcU32 Idx = 0; Idx < LAYER_MAX; ++Idx )
	{
		TLayer& Layer = Layers_[ Idx ];

		BcVec2d LayerPosition = BcVec2d( BcCos( BcPIMUL2 * Layer.TimeTicker_ ), BcSin( BcPIMUL2 * Layer.TimeTicker_ ) ) * Layer.Multiplier_;
		
		Canvas->setMaterialInstance( MaterialInstance_ );
		Canvas->drawSpriteCentered( LayerPosition, BcVec2d( 192.0f, 172.0f ), Idx, RsColour::WHITE, 0 );
	}

	Canvas->popMatrix();
}
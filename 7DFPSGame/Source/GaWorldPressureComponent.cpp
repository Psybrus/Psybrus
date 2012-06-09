/**************************************************************************
*
* File:		GaWorldPressureComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World Pressurce
*		
*
*
* 
**************************************************************************/

#include "GaWorldPressureComponent.h"

#define PROFILE_PRESSURE_UPDATE ( 0 )

//////////////////////////////////////////////////////////////////////////
// Define
DEFINE_RESOURCE( GaWorldPressureComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaWorldPressureComponent::initialise( const Json::Value& Object )
{
	Width_ = 128;
	Height_ = 128;
	Depth_ = 4;
	AccumMultiplier_ = 0.38f;
	Damping_ = 0.004f;

	Scale_ = 0.125f;
	Offset_ = BcVec2d( Width_ * Scale_, Height_ * Scale_ ) * -0.5f;

	BufferSize_ = Width_ * Height_ * Depth_;
	pBuffers_[ 0 ] = new GaWorldPressureSample[ BufferSize_ ];
	pBuffers_[ 1 ] = new GaWorldPressureSample[ BufferSize_ ];
	BcMemZero( pBuffers_[ 0 ], sizeof( GaWorldPressureSample ) * BufferSize_ );
	BcMemZero( pBuffers_[ 1 ], sizeof( GaWorldPressureSample ) * BufferSize_ );
	CurrBuffer_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void GaWorldPressureComponent::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void GaWorldPressureComponent::destroy()
{
	delete [] pBuffers_[ 0 ];
	delete [] pBuffers_[ 1 ];
	pBuffers_[ 0 ] = NULL;
	pBuffers_[ 1 ] = NULL;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool GaWorldPressureComponent::isReady()
{
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWorldPressureComponent::update( BcReal Tick )
{
#if PROFILE_PRESSURE_UPDATE
	BcTimer Timer;
	Timer.mark();
#endif

	const BcU32 NextBuffer = 1 - CurrBuffer_;
	const BcU32 WidthLessOne = Width_ - 1;
	const BcU32 HeightLessOne = Height_ - 1;
	const BcU32 DepthLessOne = Depth_ - 1;

	// TODO: Optimise a bit.
	for( BcU32 Z = 1; Z < DepthLessOne; ++Z )
	{
		for( BcU32 Y = 1; Y < HeightLessOne; ++Y )
		{
			for( BcU32 X = 1; X < WidthLessOne; ++X )
			{
				GaWorldPressureSample& Output( sample( NextBuffer, X, Y, Z ) );
				register BcReal Sample = sample( CurrBuffer_, X - 1, Y, Z ).Value_ +
				                         sample( CurrBuffer_, X + 1, Y, Z ).Value_ +
				                         sample( CurrBuffer_, X, Y - 1, Z ).Value_ +
				                         sample( CurrBuffer_, X, Y + 1, Z ).Value_ +
				                         sample( CurrBuffer_, X, Y, Z - 1 ).Value_ +
				                         sample( CurrBuffer_, X, Y, Z + 1 ).Value_;
				Sample *= AccumMultiplier_;
				Sample -= Output.Value_;
				Output.Value_ = Sample - ( Sample * Damping_ );
			}
		}
	}

	// TODO: Optimise more.
	for( BcU32 Y = 1; Y < HeightLessOne; ++Y )
	{
		for( BcU32 X = 1; X < WidthLessOne; ++X )
		{
			BcVec3d Position( BcVec2d( X, Y ) * Scale_ + Offset_, 4.0f );
			if( BSP_->checkPointBack( Position ) )
			{
				for( BcU32 Z = 1; Z < DepthLessOne; ++Z )
				{
					GaWorldPressureSample& Output( sample( NextBuffer, X, Y, Z ) );
					Output.Value_ = 0.0f;
				}
			}
		}
	}

	CurrBuffer_ = NextBuffer;

#if PROFILE_PRESSURE_UPDATE
	BcReal Time = Timer.time();
	BcPrintf("GaWorldPressureComponent Time: %.2f ms\n", Time * 1000.0f);
#endif

	BcVec2d BoxSize( 0.25f, 0.25f );
	for( BcU32 Y = 1; Y < HeightLessOne; ++Y )
	{
		for( BcU32 X = 1; X < WidthLessOne; ++X )
		{
			GaWorldPressureSample& Output( sample( CurrBuffer_, X, Y, 2 ) );

			BcVec2d Position( BcVec2d( X, Y ) * Scale_ + Offset_ );
			Canvas_->drawBox( Position - BoxSize * 0.5f, Position + BoxSize * 0.5f, RsColour::WHITE * BcAbs( Output.Value_ ), 0 );
		}
	}

	static int bleh = 0;
	bleh++;
	if(bleh > 30)
	{
		sample( CurrBuffer_, Width_ / 2, Height_ / 2, 2 ).Value_ = 8.0f;
		bleh = 0.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaWorldPressureComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWorldPressureComponent::onAttach( ScnEntityWeakRef Parent )
{
	//
	Canvas_ = Parent->getComponentByType< ScnCanvasComponent >( 0 );
	BSP_ = Parent->getComponentByType< GaWorldBSPComponent >( 0 );

	ScnMaterialRef Material;
	if( CsCore::pImpl()->requestResource( "default", "default", Material ) && CsCore::pImpl()->createResource( BcName::INVALID, Material_, Material, BcErrorCode ) )
	{
		Parent->attach( Material_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldPressureComponent::onDetach( ScnEntityWeakRef Parent )
{
	Canvas_ = NULL;
	BSP_ = NULL;
	Parent->detach( Material_ );
}

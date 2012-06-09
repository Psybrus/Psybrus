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

#define PROFILE_PRESSURE_UPDATE ( 1 )

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
	Depth_ = 8;
	AccumMultiplier_ = 0.666f;
	Damping_ = 0.02f;

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

	CurrBuffer_ = NextBuffer;

#if PROFILE_PRESSURE_UPDATE
	BcReal Time = Timer.time();
	BcPrintf("GaWorldPressureComponent Time: %.2f ms\n", Time * 1000.0f);
#endif
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

}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldPressureComponent::onDetach( ScnEntityWeakRef Parent )
{

}

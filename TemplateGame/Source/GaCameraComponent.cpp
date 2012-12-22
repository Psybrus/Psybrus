/**************************************************************************
*
* File:		GaCameraComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#include "GaCameraComponent.h"

#include "System/Audiokinetic/AkEvents.h"
#include "GeneratedSoundBanks/Wwise_IDs.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaCameraComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaCameraComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Ticker_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::update( BcReal Tick )
{
	Ticker_ += Tick * 0.5f;

	BcVec3d Position( BcVec3d( BcCos( Ticker_ ), 0.25f, -BcSin( Ticker_ ) ) * 22.0f );

	// Setup entity position to render from.
	BcMat4d LookAt;
	LookAt.lookAt( Position, BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, 1.0f, 0.0f ) );
	LookAt.inverse();
	getParentEntity()->setMatrix( LookAt );

	static BcBool StartMusic = BcTrue;
	if( StartMusic )
	{
		getParentEntity()->publish( akEVT_CORE_POST, AkEventPost( AK::EVENTS::MUSIC_PLAY ), BcFalse );
		StartMusic = BcFalse;
	}
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaCameraComponent
//virtual
void GaCameraComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}


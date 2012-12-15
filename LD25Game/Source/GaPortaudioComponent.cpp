/**************************************************************************
*
* File:		GaPortaudioComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Portaudio component.
*		
*
*
* 
**************************************************************************/

#include "GaPortaudioComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaPortaudioComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaPortaudioComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// GaPortaudioComponent
//virtual
void GaPortaudioComponent::update( BcReal Tick )
{
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// GaPortaudioComponent
//virtual
void GaPortaudioComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaPortaudioComponent
//virtual
void GaPortaudioComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Don't forget to detach!
	Super::onDetach( Parent );
}


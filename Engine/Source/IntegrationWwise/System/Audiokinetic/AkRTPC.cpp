/**************************************************************************
*
* File:		AkRTPC.cpp
* Author:	Neil Richardson 
* Ver/Date:
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Audiokinetic/AkRTPC.h"

#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( AkRTPCComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void AkRTPCComponent::initialise()
{
	RTPCID_ = BcErrorCode;
	Value_ = 0.0f;
	Dirty_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void AkRTPCComponent::update( BcReal Tick )
{
	if( Dirty_ == BcTrue )
	{
		// If we have a gamme obejct, set RTPC on that, otherwise do it globally.
		if( GameObject_.isValid() )
		{
			AK::SoundEngine::SetRTPCValue( RTPCID_, Value_, GameObject_->getGameObjectID() );
		}
		else
		{
			AK::SoundEngine::SetRTPCValue( RTPCID_, Value_, AK_INVALID_GAME_OBJECT );
		}
		Dirty_ = BcFalse;
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void AkRTPCComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
	GameObject_ = Parent->getComponentByType< AkGameObjectComponent >( 0 );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void AkRTPCComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
	GameObject_ = NULL;
}

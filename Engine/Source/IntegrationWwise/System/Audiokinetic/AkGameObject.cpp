/**************************************************************************
*
* File:		AkGameObject.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Audiokinetic/AkGameObject.h"
#include "System/Audiokinetic/AkCore.h"

#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( AkGameObjectComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void AkGameObjectComponent::initialise()
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void AkGameObjectComponent::update( BcReal Tick )
{
	Super::update( Tick );

	if( AkCore::pImpl() )
	{
		const BcMat4d& Matrix( getParentEntity()->getMatrix() );
		BcMat4d RotationMatrix = Matrix;
		RotationMatrix.translation( BcVec3d( 0.0f, 0.0f, 0.0f ) );
		BcVec3d Forward( 0.0f, 0.0f, 1.0f );
		Forward = Forward * RotationMatrix;

		AkSoundPosition SoundPosition;
		SoundPosition.Position.X = Matrix.translation().x();
		SoundPosition.Position.Y = Matrix.translation().y();
		SoundPosition.Position.Z = Matrix.translation().z();
		SoundPosition.Orientation.X = Forward.x();
		SoundPosition.Orientation.Y = Forward.y();
		SoundPosition.Orientation.Z = Forward.z();
		AK::SoundEngine::SetPosition( getGameObjectID(), SoundPosition );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void AkGameObjectComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	if( AkCore::pImpl() )
	{
		AK::SoundEngine::RegisterGameObj( getGameObjectID(), getFullName().c_str() );

		// Subscribe to events we can receive from game.
		AkEventPost::Delegate OnEventPost = AkEventPost::Delegate::bind< AkGameObjectComponent, &AkGameObjectComponent::onEventPost >( this );
		AkEventSetRTPC::Delegate OnEventSetRTPC = AkEventSetRTPC::Delegate::bind< AkGameObjectComponent, &AkGameObjectComponent::onEventSetRTPC >( this );
		Parent->subscribe( akEVT_CORE_POST, OnEventPost );
		Parent->subscribe( akEVT_CORE_SETRTPC, OnEventSetRTPC );
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void AkGameObjectComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	if( AkCore::pImpl() )
	{
		AK::SoundEngine::UnregisterGameObj( getGameObjectID() );

		// Unsubscribe from the events.
		Parent->unsubscribe( akEVT_CORE_POST, this );
		Parent->unsubscribe( akEVT_CORE_SETRTPC, this );
	}
}

//////////////////////////////////////////////////////////////////////////
// onEventPost
eEvtReturn AkGameObjectComponent::onEventPost( EvtID ID, const AkEventPost& Event )
{
	AkPlayingID PlayingID = AK::SoundEngine::PostEvent( Event.ID_, getGameObjectID() );

	BcVerifyMsg( PlayingID != AK_INVALID_PLAYING_ID, "Unable to post Wwise event %u", Event.ID_ );

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onEventSetRTPC
eEvtReturn AkGameObjectComponent::onEventSetRTPC( EvtID ID, const AkEventSetRTPC& Event )
{
	AKRESULT Result = AK::SoundEngine::SetRTPCValue( Event.ID_, Event.Value_, getGameObjectID() );

	BcVerifyMsg( Result == AK_Success, "Unable to set Wwise RTPC Value %u", Event.ID_ );

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// getGameObjectID
AkGameObjectID AkGameObjectComponent::getGameObjectID()
{
	return AkGameObjectID( this );
}

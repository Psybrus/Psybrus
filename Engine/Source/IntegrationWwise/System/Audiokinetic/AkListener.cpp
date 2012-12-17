/**************************************************************************
*
* File:		AkListener.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Audiokinetic/AkListener.h"
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
DEFINE_RESOURCE( AkListenerComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void AkListenerComponent::initialise()
{
	Super::initialise();

	ListenerID_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void AkListenerComponent::initialise( BcU32 ListenerID )
{
	initialise();

	ListenerID_ = ListenerID;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void AkListenerComponent::update( BcReal Tick )
{
	Super::update( Tick );

	if( AkCore::pImpl() )
	{
		const BcMat4d& Matrix( getParentEntity()->getMatrix() );
		BcMat4d RotationMatrix = Matrix;
		RotationMatrix.translation( BcVec3d( 0.0f, 0.0f, 0.0f ) );
		BcVec3d Up( 0.0f, -1.0f, 0.0f );
		BcVec3d Forward( 0.0f, 0.0f, -1.0f );
		Up = Up * RotationMatrix;
		Forward = Forward * RotationMatrix;

		AkListenerPosition ListenerPosition;
		ListenerPosition.Position.X = Matrix.translation().x();
		ListenerPosition.Position.Y = Matrix.translation().y();
		ListenerPosition.Position.Z = Matrix.translation().z();
		ListenerPosition.OrientationTop.X = Up.x();
		ListenerPosition.OrientationTop.Y = Up.y();
		ListenerPosition.OrientationTop.Z = Up.z();
		ListenerPosition.OrientationFront.X = Forward.x();
		ListenerPosition.OrientationFront.Y = Forward.y();
		ListenerPosition.OrientationFront.Z = Forward.z();
		AK::SoundEngine::SetListenerPosition( ListenerPosition, ListenerID_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void AkListenerComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void AkListenerComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

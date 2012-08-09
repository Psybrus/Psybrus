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
	
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void AkGameObjectComponent::update( BcReal Tick )
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

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void AkGameObjectComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	AK::SoundEngine::RegisterGameObj( getGameObjectID(), getFullName().c_str() );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void AkGameObjectComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	AK::SoundEngine::UnregisterGameObj( getGameObjectID() );
}

//////////////////////////////////////////////////////////////////////////
// getGameObjectID
AkGameObjectID AkGameObjectComponent::getGameObjectID()
{
	return AkGameObjectID( this );
}

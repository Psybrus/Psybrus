/**************************************************************************
*
* File:		ScnSoundListenerComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	23/04/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Sound/ScnSoundListenerComponent.h"

#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSoundListenerComponent );

void ScnSoundListenerComponent::StaticRegisterClass()
{	
	ReRegisterClass< ScnSoundListenerComponent, Super >()
		.addAttribute( new ScnComponentAttribute( -2020 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSoundListenerComponent::initialise( const Json::Value& Object )
{
	Super::initialise();

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnSoundListenerComponent::postUpdate( BcF32 Tick )
{
	Super::postUpdate( Tick );

	if( SsCore::pImpl() != NULL )
	{
		MaVec3d Up( 0.0f, 1.0f, 0.0f );
		MaVec3d Forward( 0.0f, 0.0f, 1.0f );
		MaMat4d Matrix = getParentEntity()->getWorldMatrix();
		MaMat4d RotationMatrix = Matrix;
		RotationMatrix.translation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
		Up = Up * RotationMatrix;
		Forward = Forward * RotationMatrix;
		//SsCore::pImpl()->setListener( Matrix.translation(), Forward, Up );
	}
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundListenerComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSoundListenerComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

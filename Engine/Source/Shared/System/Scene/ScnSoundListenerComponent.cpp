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

#include "System/Scene/ScnSoundListenerComponent.h"

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

BCREFLECTION_EMPTY_REGISTER( ScnSoundListenerComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnSoundListenerComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

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
		BcVec3d Up( 0.0f, 1.0f, 0.0f );
		BcVec3d Forward( 0.0f, 0.0f, 1.0f );
		BcMat4d Matrix = getParentEntity()->getWorldMatrix();
		BcMat4d RotationMatrix = Matrix;
		RotationMatrix.translation( BcVec3d( 0.0f, 0.0f, 0.0f ) );
		Up = Up * RotationMatrix;
		Forward = Forward * RotationMatrix;
		SsCore::pImpl()->setListener( Matrix.translation(), Forward, Up );
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

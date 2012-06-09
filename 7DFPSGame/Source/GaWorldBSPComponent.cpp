/**************************************************************************
*
* File:		GaWorldBSPComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World BSP
*		
*
*
* 
**************************************************************************/

#include "GaWorldBSPComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define
DEFINE_RESOURCE( GaWorldBSPComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaWorldBSPComponent::initialise( const Json::Value& Object )
{

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void GaWorldBSPComponent::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void GaWorldBSPComponent::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool GaWorldBSPComponent::isReady()
{
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaWorldBSPComponent::update( BcReal Tick )
{

}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaWorldBSPComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaWorldBSPComponent::onAttach( ScnEntityWeakRef Parent )
{

}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaWorldBSPComponent::onDetach( ScnEntityWeakRef Parent )
{

}

/**************************************************************************
*
* File:		GaTopState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Game top state.
*		
*
*
* 
**************************************************************************/

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaTopState::GaTopState()
{
	name( "GaTopState" );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaTopState::~GaTopState()
{
}

////////////////////////////////////////////////////////////////////////////////
// enterOnce
void GaTopState::enterOnce()
{
	pPackage_ = CsCore::pImpl()->requestPackage( "default" );
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	// omg hax. add package is ready stuff.
	return pPackage_->isReady() ? sysSR_FINISHED : sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaTopState::preMain()
{ 
	ScnEntityRef Entity;
	ScnEntityRef TemplateEntity;

	// Create screen entity.
 	if( CsCore::pImpl()->requestResource( "default", "ScreenEntity", TemplateEntity ) && CsCore::pImpl()->createResource( "ScreenEntity_0", Entity, TemplateEntity ) )
	{
		//
		ScnCore::pImpl()->addEntity( Entity );
	}

	// Create world entity.
	if( CsCore::pImpl()->requestResource( "default", "WorldEntity", TemplateEntity ) && CsCore::pImpl()->createResource( "WorldEntity_0", Entity, TemplateEntity ) )
	{
		// Setup entity position to render from.
		Entity->setPosition( BcVec3d( 0.0f, 0.0f, 0.0f ) );

		//
		ScnCore::pImpl()->addEntity( Entity );
	}
}

////////////////////////////////////////////////////////////////////////////////
// main
eSysStateReturn GaTopState::main()
{

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preLeave
void GaTopState::preLeave()
{
	ScnCore::pImpl()->removeAllEntities();
}

////////////////////////////////////////////////////////////////////////////////
// leave
eSysStateReturn GaTopState::leave()
{
	return sysSR_FINISHED;
}

////////////////////////////////////////////////////////////////////////////////
// leaveOnce
void GaTopState::leaveOnce()
{

}

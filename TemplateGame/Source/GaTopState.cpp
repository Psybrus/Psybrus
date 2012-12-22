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

	// Create paddle entity.
	if( CsCore::pImpl()->requestResource( "default", "PaddleEntity", TemplateEntity ) && CsCore::pImpl()->createResource( "PaddleEntity_0", Entity, TemplateEntity ) )
	{
		// Setup entity position to render from.
		Entity->setPosition( BcVec3d( -16.0f, 0.0f, 0.0f ) );

		//
		ScnCore::pImpl()->addEntity( Entity );
	}

	// Create paddle entity.
	if( CsCore::pImpl()->requestResource( "default", "PaddleEntity", TemplateEntity ) && CsCore::pImpl()->createResource( "PaddleEntity_1", Entity, TemplateEntity ) )
	{
		// Setup entity position to render from.
		Entity->setPosition( BcVec3d( 16.0f, 0.0f, 0.0f ) );

		//
		ScnCore::pImpl()->addEntity( Entity );
	}

	// Create ball entity.
	if( CsCore::pImpl()->requestResource( "default", "BallEntity", TemplateEntity ) && CsCore::pImpl()->createResource( "BallEntity_0", Entity, TemplateEntity ) )
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
	// NEILO HACK: Need to rework the engine init/shutdown code.
	static int LeaveTimer = 10;
	LeaveTimer--;

	CsCore::pImpl()->freeUnreferencedPackages();

	return LeaveTimer == 0 ? sysSR_FINISHED : sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// leaveOnce
void GaTopState::leaveOnce()
{
	
}

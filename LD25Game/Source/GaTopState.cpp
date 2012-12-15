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
	ScnEntityRef ScreenEntity = ScnCore::pImpl()->createEntity( "default", "ScreenEntity", "ScreenEntity_0" );
	ScnEntityRef GameEntity = ScnCore::pImpl()->createEntity( "default", "GameEntity", "GameEntity_0" );

	ScnCore::pImpl()->addEntity( ScreenEntity );
	ScnCore::pImpl()->addEntity( GameEntity );
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

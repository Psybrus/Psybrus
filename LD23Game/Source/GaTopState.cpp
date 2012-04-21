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

	ScnCore::pImpl()->addEntity( ScnCore::pImpl()->createEntity( "default", "ScreenEntity", "ScreenEntity_0" ) );
	ScnCore::pImpl()->addEntity( ScnCore::pImpl()->createEntity( "default", "GameEntity", "GameEntity_0" ) );
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

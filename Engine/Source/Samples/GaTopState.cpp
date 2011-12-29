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
	ResourceList_.push_back( CsResourceRef<>( ScnMaterial::Default ) );
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	BcBool Ready = BcTrue;

	for( BcU32 Idx = 0; Idx < ResourceList_.size(); ++Idx )
	{
		Ready &= ResourceList_[ Idx ].isReady();
	}
	
	// Wait for default material to be ready.
	if( Ready == BcTrue )
	{
		//
		return sysSR_FINISHED;
	}

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaTopState::preMain()
{ 
	ScnEntityRef Entity;
	ScnCanvasComponentRef CanvasComponent;
	ScnMaterialComponentRef MaterialComponent;

	// NOTE: This stuff will be declared in Json, imported, and support duplication.
	// Create an entity, material component, and canvas component.
	CsCore::pImpl()->createResource( "CanvasEntity_0", Entity );
	CsCore::pImpl()->createResource( BcName::INVALID, MaterialComponent, ScnMaterial::Default, scnSPF_DEFAULT );
	CsCore::pImpl()->createResource( BcName::INVALID, CanvasComponent, 8192, MaterialComponent );

	// Attach material and canvas component to entity.
	Entity->attach( MaterialComponent );
	Entity->attach( CanvasComponent );

	// Add entity to scene.
	ScnCore::pImpl()->addEntity( Entity );
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
	ResourceList_.clear();
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

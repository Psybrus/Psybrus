/**************************************************************************
*
* File:		ScnCore.cpp
* Author:	Neil Richardson 
* Ver/Date:	23/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnCore.h"

#include "SysKernel.h"
#include "SysSystem.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( ScnCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnCore::ScnCore()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnCore::~ScnCore()
{
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void ScnCore::open()
{
	// Create spacial tree.
	pSpacialTree_ = new ScnSpacialTree();

	// Create root node.
	BcVec3d HalfBounds( BcVec3d( 16.0f, 16.0f, 16.0f ) * 1024.0f );
	pSpacialTree_->createRoot( BcAABB( -HalfBounds, HalfBounds ) );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnCore::update()
{
	BcReal Tick = SysKernel::pImpl()->getFrameTime();

	// Update all entities.
	for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		Entity->update( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void ScnCore::close()
{
	// Destroy spacial tree.
	delete pSpacialTree_;
	pSpacialTree_ = NULL;
}
		

//////////////////////////////////////////////////////////////////////////
// addEntity
void ScnCore::addEntity( ScnEntityRef Entity )
{
	Entity->onAttachScene();
	pSpacialTree_->addEntity( ScnEntityWeakRef( Entity ) );
	EntityList_.push_back( Entity );

}

//////////////////////////////////////////////////////////////////////////
// removeEntity
void ScnCore::removeEntity( ScnEntityRef Entity )
{
	Entity->onDetachScene();
	pSpacialTree_->removeEntity( ScnEntityWeakRef( Entity ) );
	EntityList_.remove( Entity );
}
	
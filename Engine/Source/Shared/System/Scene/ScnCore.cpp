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

#include "OsCore.h"
#include "RsCore.h"

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
	// Tick all entities.
	BcReal Tick = SysKernel::pImpl()->getFrameTime();

	// Update all entities.
	for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		if( Entity.isReady() ) // HACK. Put in a list along side the main one to test.
		{
			Entity->update( Tick );
		}
	}

	// NEILO TODO: Move this away from here. Should be managed by ScnView and ScnSpacialTree.
	// Render to all clients.
	for( BcU32 Idx = 0; Idx < OsCore::pImpl()->getNoofClients(); ++Idx )
	{
		// Grab client.
		OsClient* pClient = OsCore::pImpl()->getClient( Idx );

		// Get context.
		RsContext* pContext = RsCore::pImpl()->getContext( pClient );

		// Allocate a frame to render using default context.
		RsFrame* pFrame = RsCore::pImpl()->allocateFrame( pContext );

		// Setup render target and viewport.
		pFrame->setRenderTarget( NULL );
		pFrame->setViewport( RsViewport( 0, 0, pClient->getWidth(), pClient->getHeight() ) );

		for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
		{
			ScnEntityRef& Entity( *It );

			if( Entity.isReady() ) // HACK. Put in a list along side the main one to test.
			{
				Entity->render( pFrame, RsRenderSort( 0 ) );
			}
		}
				
		// Queue frame for render.
		RsCore::pImpl()->queueFrame( pFrame );
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
	
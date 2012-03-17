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
	pSpacialTree_ = new ScnSpatialTree();

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

	// Do add/remove.
	processAddRemove();

	// Update all entities.
	for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		if( Entity.isReady() ) // HACK. Put in a list along side the main one to test.
		{
			Entity->update( Tick );
		}
	}

	// Render to all clients.
	// TODO: Move client/context into the view component instead.
	// TODO: Use spatial tree to render.
	for( BcU32 Idx = 0; Idx < OsCore::pImpl()->getNoofClients(); ++Idx )
	{
		// Grab client.
		OsClient* pClient = OsCore::pImpl()->getClient( Idx );

		// Get context.
		RsContext* pContext = RsCore::pImpl()->getContext( pClient );

		// Allocate a frame to render using default context.
		RsFrame* pFrame = RsCore::pImpl()->allocateFrame( pContext );

		// Iterate over all view components.
		for( ScnViewComponentListIterator It( ViewComponentList_.begin() ); It != ViewComponentList_.end(); ++It )
		{
			ScnViewComponentRef ViewComponent( *It );
			
			ViewComponent->bind( pFrame, RsRenderSort( 0 ) );
			
			for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
			{
				ScnEntityRef& Entity( *It );

				if( Entity.isReady() ) // HACK. Put in a list along side the main one to test.
				{
					for( BcU32 ComponentIdx = 0; ComponentIdx < Entity->getNoofComponents(); ++ComponentIdx )
					{
						ScnMaterialComponentRef MaterialComponent( Entity->getComponent( ComponentIdx ) );

						if( MaterialComponent.isValid() )
						{
							ViewComponent->setMaterialParameters( MaterialComponent );
						}
					}
					
					Entity->render( pFrame, RsRenderSort( 0 ) );
				}
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
	if( !Entity->isAttached() )
	{
		AddEntityList_.remove( Entity );
		AddEntityList_.push_back( Entity );
	}
}

//////////////////////////////////////////////////////////////////////////
// removeEntity
void ScnCore::removeEntity( ScnEntityRef Entity )
{
	if( Entity->isAttached() )
	{
		RemoveEntityList_.remove( Entity );
		RemoveEntityList_.push_back( Entity );
	}
}

//////////////////////////////////////////////////////////////////////////
// removeAllEntities
void ScnCore::removeAllEntities()
{
	for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );
		removeEntity( Entity );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
void ScnCore::onAttachComponent( ScnEntityWeakRef Entity, ScnComponentRef Component )
{
	// NOTE: Useful for debugging and temporary gathering of "special" components.
	//       Will be considering alternative approaches to this.
	//       Currently, just gonna be nasty special cases to get stuff done.
	
	// Add view components for render usage.
	if( Component->isTypeOf< ScnViewComponent >() )
	{
		ViewComponentList_.push_back( ScnViewComponentRef( Component ) );
	}
	// Add renderable components to the spatial tree. (TODO: Use flags or something)
	else if( Component->isTypeOf< ScnRenderableComponent >() )
	{
		pSpacialTree_->addComponent( ScnComponentWeakRef( Component ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
void ScnCore::onDetachComponent( ScnEntityWeakRef Entity, ScnComponentRef Component )
{
	// NOTE: Useful for debugging and temporary gathering of "special" components.
	//       Will be considering alternative approaches to this.
	//       Currently, just gonna be nasty special cases to get stuff done.

	// Remove view components for render usage.
	if( Component->isTypeOf< ScnViewComponent >() )
	{
		ViewComponentList_.remove( ScnViewComponentRef( Component ) );
	}
	// Add renderable components to the spatial tree. (TODO: Use flags or something)
	else if( Component->isTypeOf< ScnRenderableComponent >() )
	{
		pSpacialTree_->removeComponent( ScnComponentWeakRef( Component ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// processAddRemove
void ScnCore::processAddRemove()
{
	for( ScnEntityListIterator It( RemoveEntityList_.begin() ); It != RemoveEntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );
		Entity->onDetachScene();
		EntityList_.remove( Entity );

		// Do onDetachComponent for all entities current components.
		for( BcU32 Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
		{
			ScnComponentRef Component( Entity->getComponent( Idx ) );
			onDetachComponent( ScnEntityWeakRef( Entity ), Component );
			Component->onDetach( ScnEntityWeakRef( Entity ) );				// HACK? I don't think this should be called. Entities are still attached to the entity.		
		}
	}
	RemoveEntityList_.clear();

	for( ScnEntityListIterator It( AddEntityList_.begin() ); It != AddEntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		Entity->onAttachScene();
		EntityList_.push_back( Entity );

		// Do onAttachComponent for all entities current components.
		for( BcU32 Idx = 0; Idx < Entity->getNoofComponents(); ++Idx )
		{
			onAttachComponent( ScnEntityWeakRef( Entity ), Entity->getComponent( Idx ) );
		}
	}
	AddEntityList_.clear();
}
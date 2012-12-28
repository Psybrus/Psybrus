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

#include "System/Scene/ScnCore.h"

#include "System/SysKernel.h"
#include "System/SysSystem.h"

#include "System/Os/OsCore.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsCore.h"

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
	BcF32 Tick = SysKernel::pImpl()->getFrameTime();

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

				if( Entity.isReady() && ( Entity->getRenderMask() & ViewComponent->getRenderMask() ) != 0 ) // HACK. Put in a list along side the main one to test.
				{				
					Entity->render( ViewComponent, pFrame, RsRenderSort( 0 ) );
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
	CsCore* pCore = CsCore::pImpl();

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
	CsCore* pCore = CsCore::pImpl();

	for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );
		removeEntity( Entity );
	}
}

//////////////////////////////////////////////////////////////////////////
// createEntity
ScnEntityRef ScnCore::createEntity(  const BcName& Package, const BcName& Name, const BcName& InstanceName )
{
	ScnEntityRef Entity;
	ScnEntityRef TemplateEntity;

	// Request template entity.
 	if( CsCore::pImpl()->requestResource( Package, Name, TemplateEntity ) )
	{
		BcName UniqueName = Name.getUnique();
		CsPackage* pPackage = CsCore::pImpl()->findPackage( Package );
		if( CsCore::pImpl()->createResource( InstanceName == BcName::INVALID ? UniqueName : InstanceName, pPackage, Entity, TemplateEntity ) )
		{
			return Entity;
		}
	}

	BcAssertMsg( BcFalse, "ScnCore: Can't create entity \"%s\" from \"$(ScnEntity:%s.%s)\"", (*InstanceName).c_str(), (*Package).c_str(), (*Name).c_str() );

	return NULL;	
}

//////////////////////////////////////////////////////////////////////////
// findEntity
ScnEntityRef ScnCore::findEntity( const BcName& InstanceName )
{
	for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		if( Entity->getName() == InstanceName )
		{
			return Entity;
		}
	}

	for( ScnEntityListIterator It( AddEntityList_.begin() ); It != AddEntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		if( Entity->getName() == InstanceName )
		{
			return Entity;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getEntity
ScnEntityRef ScnCore::getEntity( BcU32 Idx )
{
	// NOTE: Should probably switch to std::vector do I don't need to do this bullshit.
	BcU32 TotalIdx = 0;
	for( ScnEntityListIterator It( EntityList_.begin() ); It != EntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		if( TotalIdx++ == Idx )
		{
			return Entity;
		}
	}

	return ScnEntityRef( NULL );
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
		Entity->onDetach( NULL );
		EntityList_.remove( Entity );
	}
	RemoveEntityList_.clear();

	for( ScnEntityListIterator It( AddEntityList_.begin() ); It != AddEntityList_.end(); ++It )
	{
		ScnEntityRef Entity( *It );

		Entity->onAttach( NULL );
		EntityList_.push_back( Entity );
	}
	AddEntityList_.clear();
}

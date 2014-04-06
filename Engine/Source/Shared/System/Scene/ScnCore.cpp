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

#include "System/Scene/ScnSpatialComponent.h"

#include "System/Scene/ScnRenderingVisitor.h"

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( ScnCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnCore::ScnCore()
{
	pSpatialTree_ = NULL;
	pComponentLists_ = NULL;
	NoofComponentLists_ = 0;
	EntitySpawnID_ = 0;
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
	pSpatialTree_ = new ScnSpatialTree();

	// Create root node for spatial tree.
	BcVec3d HalfBounds( BcVec3d( 16.0f, 16.0f, 16.0f ) * 1024.0f );
	pSpatialTree_->createRoot( BcAABB( -HalfBounds, HalfBounds ) );

	// Look up all component classes and create update lists for them.
	NoofComponentLists_ = 0;
	BcU32 NoofClasses = BcReflection::pImpl()->getNoofClasses();
	for( BcU32 Idx = 0; Idx < NoofClasses; ++Idx )
	{
		const BcReflectionClass* pClass = BcReflection::pImpl()->getClass( Idx );
		if( pClass->isTypeOfClass( ScnComponent::StaticGetClass() ) )
		{
			ComponentClassIndexMap_[ pClass ] = NoofComponentLists_++;
		}
	}

	pComponentLists_ = new ScnComponentList[ NoofComponentLists_ ];	 
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnCore::update()
{
	// Tick all entities.
	BcF32 Tick = SysKernel::pImpl()->getFrameTime();

	// Pre-update.
	for( BcU32 ListIdx = 0; ListIdx < NoofComponentLists_; ++ListIdx )
	{
		ScnComponentList& ComponentList( pComponentLists_[ ListIdx ] );

		for( ScnComponentListIterator It( ComponentList.begin() ); It != ComponentList.end(); ++It )
		{
			ScnComponentRef Component( *It );

			BcAssert( Component.isReady() );
			Component->preUpdate( Tick );
		}
	}

	// Update.
	for( BcU32 ListIdx = 0; ListIdx < NoofComponentLists_; ++ListIdx )
	{
		ScnComponentList& ComponentList( pComponentLists_[ ListIdx ] );

		for( ScnComponentListIterator It( ComponentList.begin() ); It != ComponentList.end(); ++It )
		{
			ScnComponentRef Component( *It );

			BcAssert( Component.isReady() );
			Component->update( Tick );
		}
	}

	// Post-update.
	for( BcU32 ListIdx = 0; ListIdx < NoofComponentLists_; ++ListIdx )
	{
		ScnComponentList& ComponentList( pComponentLists_[ ListIdx ] );

		// Temporary hack to iron out some minor issues.
		ScnComponentList CopiedComponentList = ComponentList;

		for( ScnComponentListIterator It( ComponentList.begin() ); It != ComponentList.end(); ++It )
		{
			ScnComponentRef Component( *It );

			BcAssert( Component.isReady() );
			Component->postUpdate( Tick );
		}
	}

	// Render to all clients.
	// TODO: Move client/context into the view component instead.
	// TODO: Move the whole render process into the view component.
	//       - Perhaps we want to have frames allocated per client?
	//         Doing this means we can have the actual queueFrame call
	//         inside the renderer, and not be a fart on here.
	//         Also, the view component should be aware of the frame
	//         and provide access to it for renderable components.
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

			ScnRenderingVisitor Visitor( ViewComponent, pFrame );
		}

		// Queue frame for render.
		RsCore::pImpl()->queueFrame( pFrame );
	}

	// Process pending components at the end of the tick.
	// We do this because they can be immediately created,
	// and need a create tick from CsCore next frame.
	processPendingComponents();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void ScnCore::close()
{
	removeAllEntities();
	processPendingComponents();

	delete [] pComponentLists_;
	pComponentLists_ = NULL;

	// Destroy spacial tree.
	delete pSpatialTree_;
	pSpatialTree_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// addEntity
void ScnCore::addEntity( ScnEntityRef Entity )
{
	Entity->setFlag( scnCF_PENDING_ATTACH );
	queueComponentAsPendingOperation( ScnComponentRef( Entity ) );
}

//////////////////////////////////////////////////////////////////////////
// removeEntity
void ScnCore::removeEntity( ScnEntityRef Entity )
{
	if(Entity->getParentEntity() == NULL )
	{
		Entity->setFlag( scnCF_PENDING_DETACH );
		queueComponentAsPendingOperation( ScnComponentRef( Entity ) );
	}
	else
	{
		Entity->getParentEntity()->detach( Entity );
	}
}

//////////////////////////////////////////////////////////////////////////
// removeAllEntities
void ScnCore::removeAllEntities()
{
	BcU32 ComponentListIdx( ComponentClassIndexMap_[ ScnEntity::StaticGetClass() ] );
	ScnComponentList& ComponentList( pComponentLists_[ ComponentListIdx ] );
	for( ScnComponentListIterator It( ComponentList.begin() ); It != ComponentList.end(); ++It )
	{
		ScnComponentRef Component( *It );
		ScnEntityRef Entity( Component );
		// Only remove root entities, it will cascade down the hierarchy removing them.
		if( Entity->getParentEntity() != NULL )
		{
			removeEntity( Entity );
		}
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

	BcAssertMsg( BcFalse, "ScnCore: Can't create entity \"%s\" from \"%s.%s:%s\"", (*InstanceName).c_str(), (*Package).c_str(), (*Name).c_str(), "ScnEntity" );

	return NULL;	
}

//////////////////////////////////////////////////////////////////////////
// spawnEntity
ScnEntity* ScnCore::spawnEntity( const ScnEntitySpawnParams& Params )
{
	BcAssert( BcIsGameThread() );

	// Get package and acquire.
	CsPackage* pPackage = CsCore::pImpl()->requestPackage( Params.Package_ );

	if(pPackage->isReady())
	{
		return internalSpawnEntity( Params );
	}
	else
	{
		pPackage->acquire();
	
		// Register for ready callback.
		EntitySpawnMap_[ EntitySpawnID_ ] = Params;
		CsCore::pImpl()->requestPackageReadyCallback( Params.Package_, CsPackageReadyCallback::bind< ScnCore, &ScnCore::onSpawnEntityPackageReady >( this ), EntitySpawnID_ );

		// Advance spawn ID.
		++EntitySpawnID_;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// findEntity
ScnEntityRef ScnCore::findEntity( const BcName& InstanceName )
{
	BcU32 ComponentListIdx( ComponentClassIndexMap_[ ScnEntity::StaticGetClass() ] );
	ScnComponentList& ComponentList( pComponentLists_[ ComponentListIdx ] );
	for( ScnComponentListIterator It( ComponentList.begin() ); It != ComponentList.end(); ++It )
	{
		ScnComponentRef Component( *It );
		ScnEntityRef Entity( Component );
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
	BcU32 ComponentListIdx( ComponentClassIndexMap_[ ScnEntity::StaticGetClass() ] );
	ScnComponentList& ComponentList( pComponentLists_[ ComponentListIdx ] );

	if( Idx < ComponentList.size() )
	{
		return ScnEntityRef( ComponentList[ Idx ] );
	}

	return ScnEntityRef( NULL );
}

//////////////////////////////////////////////////////////////////////////
// queueComponentAsPendingOperation
void ScnCore::queueComponentAsPendingOperation( ScnComponentRef Component )
{
	PendingComponentList_.push_back( Component );
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnCore::visitView( ScnVisitor* pVisitor, const ScnViewComponent* View )
{
	pSpatialTree_->visitView( pVisitor, View );
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnCore::visitBounds( class ScnVisitor* pVisitor, const BcAABB& Bounds )
{
	pSpatialTree_->visitBounds( pVisitor, Bounds );
}

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
void ScnCore::onAttachComponent( ScnEntityWeakRef Entity, ScnComponent* Component )
{
	// NOTE: Useful for debugging and temporary gathering of "special" components.
	//       Will be considering alternative approaches to this.
	//       Currently, just gonna be nasty special cases to get stuff done.
	
	// Add view components for render usage.
	if( Component->isTypeOf< ScnViewComponent >() )
	{
		ViewComponentList_.push_back( static_cast< ScnViewComponent* >( Component ) );
	}
	// Add spatial components to the spatial tree. (TODO: Use flags or something)
	else if( Component->isTypeOf< ScnSpatialComponent >() )
	{
		pSpatialTree_->addComponent( static_cast< ScnSpatialComponent* >( Component ) );
	}

	// All go into the appropriate list.
	const BcReflectionClass* pClass = Component->getClass();
	BcU32 Idx( ComponentClassIndexMap_[ pClass ] );
	ScnComponentList& ComponentList( pComponentLists_[ Idx ] );
	ComponentList.push_back( Component );
}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
void ScnCore::onDetachComponent( ScnEntityWeakRef Entity, ScnComponent* Component )
{
	// NOTE: Useful for debugging and temporary gathering of "special" components.
	//       Will be considering alternative approaches to this.
	//       Currently, just gonna be nasty special cases to get stuff done.
	// NOTE: Now that we have component type lists, we don't need to be specific with this.

	// Remove view components for render usage.
	if( Component->isTypeOf< ScnViewComponent >() )
	{
		ViewComponentList_.remove( static_cast< ScnViewComponent* >( Component ) );
	}
	// Add renderable components to the spatial tree. (TODO: Use flags or something)
	else if( Component->isTypeOf< ScnSpatialComponent >() )
	{
		pSpatialTree_->removeComponent( static_cast< ScnSpatialComponent* >( Component ) );
	}

	// Erase from component list.
	const BcReflectionClass* pClass = Component->getClass();
	BcU32 Idx( ComponentClassIndexMap_[ pClass ] );
	ScnComponentList& ComponentList( pComponentLists_[ Idx ] );
	ScnComponentListIterator It = std::find( ComponentList.begin(), ComponentList.end(), Component );
	ComponentList.erase( It );
}

//////////////////////////////////////////////////////////////////////////
// processPendingComponents
void ScnCore::processPendingComponents()
{
	while( PendingComponentList_.size() > 0 )
	{
		//
		ScnComponentRef Component( *PendingComponentList_.begin() );
		PendingComponentList_.erase( PendingComponentList_.begin() );

		if( Component->isFlagSet( scnCF_PENDING_ATTACH ) )
		{
			Component->onAttach( Component->getParentEntity() );
			onAttachComponent( ScnEntityWeakRef( Component->getParentEntity() ), ScnComponentRef( Component ) );
		}
		
		if( Component->isFlagSet( scnCF_PENDING_DETACH ) )
		{
			Component->onDetach( Component->getParentEntity() );
			onDetachComponent( ScnEntityWeakRef( Component->getParentEntity() ), ScnComponentRef( Component ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// internalSpawnEntity
ScnEntity* ScnCore::internalSpawnEntity( ScnEntitySpawnParams Params )
{
	// Create entity.
	ScnEntityRef Entity = createEntity( Params.Package_, Params.Name_, Params.InstanceName_ );

	// Set it's transform.
	Entity->setLocalMatrix( Params.Transform_ );

	// If we have a valid parent, attach to it. Otherwise, add to the scene root.
	if( Params.Parent_.isValid() )
	{
		Params.Parent_->attach( Entity );
	}
	else
	{
		addEntity( Entity );
	}

	return Entity;
}

//////////////////////////////////////////////////////////////////////////
// onSpawnEntityPackageReady
void ScnCore::onSpawnEntityPackageReady( CsPackage* pPackage, BcU32 ID )
{
	TEntitySpawnDataMapIterator It = EntitySpawnMap_.find( ID );
	BcAssertMsg( It != EntitySpawnMap_.end(), "ScnCore: Spawn ID invalid." );
	ScnEntitySpawnParams& EntitySpawnData( (*It).second );

	// Spawn!
	internalSpawnEntity( EntitySpawnData );

	// Release package, the entity is responsible for it now.
	pPackage->release();

	// Clear out the spawn data.
	EntitySpawnMap_.erase( It );
}

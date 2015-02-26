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

#include "System/Scene/ScnSpatialTree.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Scene/ScnSpatialComponent.h"
#include "System/Scene/Rendering/ScnRenderingVisitor.h"

#include "Base/BcProfiler.h"

#include <algorithm>

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
	MaVec3d HalfBounds( MaVec3d( 16.0f, 16.0f, 16.0f ) * 1024.0f );
	pSpatialTree_->createRoot( MaAABB( -HalfBounds, HalfBounds ) );

	// Look up all component classes and create update lists for them.
	NoofComponentLists_ = 0;
	typedef std::pair< ReClass*, BcS32 > ComponentPriorityPair;
	typedef std::vector< ComponentPriorityPair > ComponentClasses;
	ComponentClasses ComponentClasses_;

	// Extract all the classes with the right attribute.
	auto Classes = ReManager::GetClasses();
	for( auto Class : Classes )
	{
		auto* Attr = Class->getAttribute< ScnComponentAttribute >();
		if( Attr != nullptr )
		{
			ComponentClasses_.push_back( std::make_pair( Class, Attr->getUpdatePriority() ) );
		}
	}

	// Sort by their update priority.
	std::sort( ComponentClasses_.begin(), ComponentClasses_.end(), 
		[] ( ComponentPriorityPair A, ComponentPriorityPair B )
		{
			return A.second < B.second;
		}
	);

	// Write list index map out.
	for( auto ComponentClass : ComponentClasses_ )
	{
		ComponentClassIndexMap_[ ComponentClass.first ] = NoofComponentLists_++;
	}

	BcAssert( NoofComponentLists_ > 0 );

	pComponentLists_ = new ScnComponentList[ NoofComponentLists_ ];	 
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnCore::update()
{
	PSY_PROFILER_SECTION( UpdateRoot, std::string( "ScnCore::update" ) );

	// Tick all entities.
	BcF32 Tick = SysKernel::pImpl()->getFrameTime();

	// Pre-update.
	for( BcU32 ListIdx = 0; ListIdx < NoofComponentLists_; ++ListIdx )
	{
		ScnComponentList& ComponentList( pComponentLists_[ ListIdx ] );

		for( ScnComponentListIterator It( ComponentList.begin() ); It != ComponentList.end(); ++It )
		{
			ScnComponentRef Component( *It );

			BcAssert( Component.isValid() && Component->isReady() );
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

			BcAssert( Component.isValid() && Component->isReady() );
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

			BcAssert( Component.isValid() && Component->isReady() );
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
		PSY_PROFILER_SECTION( RenderRoot, std::string( "ScnCore::render" ) );

		// Grab client.
		OsClient* pClient = OsCore::pImpl()->getClient( Idx );

		// Get context.
		RsContext* pContext = RsCore::pImpl()->getContext( pClient );

		// Allocate a frame to render using default context.
		RsFrame* pFrame = RsCore::pImpl()->allocateFrame( pContext );

		// Iterate over all view components.
		RsRenderSort Sort( 0 );
		BcAssert( ViewComponentList_.size() < RS_SORT_VIEWPORT_MAX );
		for( ScnComponentListIterator It( ViewComponentList_.begin() ); It != ViewComponentList_.end(); ++It )
		{
			ScnViewComponentRef ViewComponent( *It );
			
			ViewComponent->bind( pFrame, Sort );

			ScnRenderingVisitor Visitor( ViewComponent, pFrame, Sort );

			// Increment viewport.
			Sort.Viewport_++;
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
	pComponentLists_ = nullptr;

	// Destroy spacial tree.
	delete pSpatialTree_;
	pSpatialTree_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// addEntity
void ScnCore::addEntity( ScnEntityRef Entity )
{
	BcAssert( Entity->getName().isValid() );
	BcAssert( !Entity->isFlagSet( scnCF_ATTACHED ) );
	Entity->setFlag( scnCF_PENDING_ATTACH );
	queueComponentAsPendingOperation( ScnComponentRef( Entity ) );
}

//////////////////////////////////////////////////////////////////////////
// removeEntity
void ScnCore::removeEntity( ScnEntityRef Entity )
{
	BcAssert( Entity->getName().isValid() );
	if( Entity->getParentEntity() == nullptr )
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
		if( Entity->getParentEntity() == nullptr )
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
			Entity->postInitialise();
			return Entity;
		}
	}

	BcAssertMsg( BcFalse, "ScnCore: Can't create entity \"%s\" from \"%s.%s:%s\"", (*InstanceName).c_str(), (*Package).c_str(), (*Name).c_str(), "ScnEntity" );

	return nullptr;	
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
		using namespace std::placeholders;
		CsCore::pImpl()->requestPackageReadyCallback( 
			Params.Package_, 
			std::bind( &ScnCore::onSpawnEntityPackageReady, this, _1, _2 ), 
			EntitySpawnID_ );

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
	
	return nullptr;
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

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// queueComponentAsPendingOperation
void ScnCore::queueComponentAsPendingOperation( ScnComponentRef Component )
{
	BcAssert( Component->getName() != BcName::INVALID );
	
	// Add pending operation if not already in queue.
	if( std::find( PendingComponentList_.begin(), PendingComponentList_.end(), Component ) == PendingComponentList_.end() )
	{
		PendingComponentList_.push_back( Component );
	}
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnCore::visitView( ScnVisitor* pVisitor, const ScnViewComponent* View )
{
	pSpatialTree_->visitView( pVisitor, View );
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnCore::visitBounds( class ScnVisitor* pVisitor, const MaAABB& Bounds )
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
	const auto* pClass = Component->getClass();
	auto FoundIndexIt = ComponentClassIndexMap_.find( pClass );
	if( FoundIndexIt != ComponentClassIndexMap_.end() )
	{
		BcU32 Idx( FoundIndexIt->second );
		ScnComponentList& ComponentList( pComponentLists_[ Idx ] );
		ComponentList.push_back( Component );
	}
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
		ViewComponentList_.erase( std::find( ViewComponentList_.begin(), ViewComponentList_.end(), static_cast< ScnViewComponent* >( Component ) ) );
	}
	// Add renderable components to the spatial tree. (TODO: Use flags or something)
	else if( Component->isTypeOf< ScnSpatialComponent >() )
	{
		pSpatialTree_->removeComponent( static_cast< ScnSpatialComponent* >( Component ) );
	}

	// Erase from component list.
	const ReClass* pClass = Component->getClass();
	auto FoundIndexIt = ComponentClassIndexMap_.find( pClass );
	if( FoundIndexIt != ComponentClassIndexMap_.end() )
	{
		BcU32 Idx( FoundIndexIt->second );
		ScnComponentList& ComponentList( pComponentLists_[ Idx ] );
		ScnComponentListIterator It = std::find( ComponentList.begin(), ComponentList.end(), Component );
		ComponentList.erase( It );
	}
}

//////////////////////////////////////////////////////////////////////////
// processPendingComponents
void ScnCore::processPendingComponents()
{
	ScnComponentList ComponentStillNotReady;
	ScnComponentList ComponentToDestroy;
	while( PendingComponentList_.size() > 0 )
	{
		//
		ScnComponentRef Component( *PendingComponentList_.begin() );
		PendingComponentList_.erase( PendingComponentList_.begin() );

		BcAssert( Component->getInitStage() != CsResource::INIT_STAGE_DESTROY );

		//
		if( Component->isReady() )
		{
			// Handle attachment.
			if( Component->isFlagSet( scnCF_PENDING_ATTACH ) )
			{
				Component->onAttach( Component->getParentEntity() );
				onAttachComponent( ScnEntityWeakRef( Component->getParentEntity() ), ScnComponentRef( Component ) );
			}

			// Handle detachment.
			if( Component->isFlagSet( scnCF_PENDING_DETACH ) )
			{
				Component->onDetach( Component->getParentEntity() );
				BcAssertMsg( Component->isFlagSet( scnCF_PENDING_DETACH ) == BcFalse, 
					"Have you called Super::onDetach in type %s?", (*Component->getTypeName()).c_str() );
				onDetachComponent( ScnEntityWeakRef( Component->getParentEntity() ), ScnComponentRef( Component ) );
			}

			// Handle destruction.
			if( Component->isFlagSet( scnCF_PENDING_DESTROY ) )
			{
				ComponentToDestroy.push_back( Component );
			}
		}
		else
		{
			ComponentStillNotReady.push_back( Component );
		}
	}

	PendingComponentList_.insert( 
		PendingComponentList_.begin(), ComponentStillNotReady.begin(), ComponentStillNotReady.end() );

	// Do destruction.
	for( auto Component : ComponentToDestroy )
	{
		Component->markDestroy();
	}
}

//////////////////////////////////////////////////////////////////////////
// internalSpawnEntity
ScnEntity* ScnCore::internalSpawnEntity( 
	ScnEntitySpawnParams Params )
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

	// Call on spawn callback.
	if( Params.OnSpawn_ != nullptr )
	{
		Params.OnSpawn_( Entity );
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

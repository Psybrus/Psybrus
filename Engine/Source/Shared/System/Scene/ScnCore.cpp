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
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "System/Scene/ScnCoreCallback.h"
#include "System/Debug/DsCore.h"

#include "System/Scene/ScnSpatialTree.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Scene/ScnSpatialComponent.h"
#include "System/Scene/Rendering/ScnRenderingVisitor.h"

#include "Reflection/ReReflection.h"

#include "Serialisation/SeJsonWriter.h"

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
	std::vector< ReClass* > ComponentClasses;

	// Extract all the classes with the right attribute.
	auto Classes = ReManager::GetClasses();
	for( auto Class : Classes )
	{
		auto* Attr = Class->getAttribute< ScnComponentProcessor >();
		if( Attr != nullptr )
		{
			// Add component class to list for later.
			ComponentClasses.push_back( Class );

			// Get process funcs.
			auto ProcessFuncs = Attr->getProcessFuncs();

			// Add to list.
			for( auto& ProcessFunc : ProcessFuncs )
			{
				ComponentProcessFuncs_.push_back( ProcessFunc );
			}
		}
	}

	// Sort process funcs by their priority.
	std::sort( ComponentProcessFuncs_.begin(), ComponentProcessFuncs_.end(), 
		[] ( ScnComponentProcessFuncEntry A, ScnComponentProcessFuncEntry B )
		{
			return A.Priority_ < B.Priority_;
		}
	);


	// Write list index map out.
	ComponentLists_.reserve( ComponentClasses.size() );
	for( auto ComponentClass : ComponentClasses )
	{
		ComponentIndexClassMap_[ ComponentLists_.size() ] = ComponentClass;
		ComponentClassIndexMap_[ ComponentClass ] = ComponentLists_.size();
		ComponentLists_.push_back( ScnComponentList() );
	}

#if !PSY_PRODUCTION
	DsCore::pImpl()->registerPanel( 
		"Scene Stats", [ this ]( BcU32 )->void
		{
			static BcF32 GameTimeTotal = 0.0f;
			static BcF32 FrameTimeTotal = 0.0f;
			static BcF32 GameTimeAccum = 0.0f;
			static BcF32 FrameTimeAccum = 0.0f;
			static int CaptureAmount = 60;
			static int CaptureAccum = 0;
			GameTimeAccum += SysKernel::pImpl()->getGameThreadTime();
			FrameTimeAccum += SysKernel::pImpl()->getFrameTime();
			++CaptureAccum;
			if( CaptureAccum >= CaptureAmount )
			{
				GameTimeTotal = GameTimeAccum / BcF32( CaptureAccum );
				FrameTimeTotal = FrameTimeAccum / BcF32( CaptureAccum );
				GameTimeAccum = 0.0f;
				FrameTimeAccum = 0.0f;
				CaptureAccum = 0;
			}

			OsClient* Client = OsCore::pImpl()->getClient( 0 );
			MaVec2d WindowPos = MaVec2d( Client->getWidth() - 300.0f, 10.0f );
			static bool ShowOpened = true;
			ImGui::SetNextWindowPos( WindowPos );
			if ( ImGui::Begin( "Scene Stats", &ShowOpened, ImVec2( 0.0f, 0.0f ), 0.3f, 
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize ) )
			{
				if( ImGui::Button( "Screenshot" ) )
				{
					RsCore::pImpl()->getContext( nullptr )->takeScreenshot();
				}

				ImGui::Text( "Worker count: %u", 
					SysKernel::pImpl()->workerCount() );
				ImGui::Text( "Game time: %.2fms (%.2fms avg.)", 
					SysKernel::pImpl()->getGameThreadTime() * 1000.0f, GameTimeTotal * 1000.0f );
				ImGui::Text( "Frame time: %.2fms (%.2fms avg.)", 
					SysKernel::pImpl()->getFrameTime() * 1000.0f, FrameTimeTotal * 1000.0f );

			}
			ImGui::End();
		} );

	DsCore::pImpl()->registerPanel(
		"Scene Hierarchy", [ this ]( BcU32 )->void
		{
			// Render scene hierarchy.
			using ComponentNodeFunc = std::function< void( ScnComponent* Component ) >;
			ComponentNodeFunc RecurseNode = 
				[ & ]( ScnComponent* Component )
				{
					ImGui::PushID( Component );
					if( Component->isTypeOf< ScnEntity >() )
					{
						auto TreeNodeOpen = ImGui::TreeNode( Component, "" );
						ImGui::SameLine();

						if( ImGui::SmallButton( (*Component->getName()).c_str() ) )
						{
							DebugComponents_.clear();
							DebugComponents_.push_back( Component );
						}	

						if( TreeNodeOpen )
						{
							BcU32 ChildIdx = 0;
							while( auto Child = Component->getComponent( ChildIdx++ ) )
							{
								RecurseNode( Child );
							}
							ImGui::TreePop();
						}
					}
					ImGui::PopID();
				};

			static bool ShowOpened = true;
			if ( ImGui::Begin( "Scene Hierarchy", &ShowOpened ) )
			{
				if( ImGui::TreeNode( "Scene Hierarchy" ) )
				{
					BcU32 Idx = 0;
					while( ScnEntityRef Entity = getEntity( Idx++ ) )
					{
						if( Entity->getParentEntity() == nullptr )
						{
							RecurseNode( Entity );
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::End();
		} );

	DsCore::pImpl()->registerPanel(
		"Component Editor", [ this ]( BcU32 )->void
		{
			if ( ImGui::Begin( "Component Editor" ) )
			{
				ImGui::Text( "Components editing: %u", DebugComponents_.size() );
				ImGui::Separator();
				for( auto Component : DebugComponents_ )
				{
					auto UpperClass = Component->getClass();
					auto Class = UpperClass;

					// Find editor.
					DsImGuiFieldEditor* FieldEditor = nullptr;
					while( FieldEditor == nullptr && Class != nullptr )
					{
						FieldEditor = Class->getAttribute< DsImGuiFieldEditor >();
						Class = Class->getSuper();

					}
					if( FieldEditor )
					{
						FieldEditor->onEdit( "", Component, UpperClass, bcRFF_NONE );
					}
				}
			}
			ImGui::End();
		} );

	DsCore::pImpl()->registerPanel(
		"Component Process Funcs", [ this ]( BcU32 )->void
		{
			if ( ImGui::Begin( "Component Process Funcs" ) )
			{
				for( auto& ComponentProcessFunc : ComponentProcessFuncs_ )
				{
					auto ComponentListIdx = ComponentClassIndexMap_[ ComponentProcessFunc.Class_ ];
					auto& ComponentList = ComponentLists_[ ComponentListIdx ];
					ImGui::Text( "%s::%s", 
						(*ComponentProcessFunc.Class_->getName()).c_str(),
						ComponentProcessFunc.Name_.c_str() );
					ImGui::Text( "- Priority : %i", 
						ComponentProcessFunc.Priority_ );
					ImGui::Text( "- Components : %u", 
						ComponentList.size() );
				}
			}
			ImGui::End();
		} );
#endif // !PSY_PRODUCTION

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnCore::update()
{
	PSY_PROFILER_SECTION( UpdateRoot, std::string( "ScnCore::update" ) );

	BcF32 Tick = SysKernel::pImpl()->getFrameTime();

	// Iterate over all component process funcs.
	for( auto& ComponentProcessFunc : ComponentProcessFuncs_ )
	{
		auto ComponentListIdx = ComponentClassIndexMap_[ ComponentProcessFunc.Class_ ];
		auto& ComponentList = ComponentLists_[ ComponentListIdx ];
		ComponentProcessFunc.Func_( ComponentList );
	}

	// Render to all clients.
	// TODO: Move client/context into the view component instead.
	// TODO: Move the whole render process into the view component.
	//       - Perhaps we want to have frames allocated per client?
	//         Doing this means we can have the actual queueFrame call
	//         inside the renderer, and not be a fart on here.
	//         Also, the view component should be aware of the frame
	//         and provide access to it for renderable components.
	if( ViewComponentList_.size() > 0 )
	{
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

			// TODO: Move completely to DsCore.
			// Render ImGui.
			ImGui::Psybrus::Render( pContext, pFrame );

			// Queue frame for render.
			RsCore::pImpl()->queueFrame( pFrame );
		}
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

	ComponentLists_.clear();

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
	queueComponentForAttach( ScnComponentRef( Entity ) );
}

//////////////////////////////////////////////////////////////////////////
// removeEntity
void ScnCore::removeEntity( ScnEntityRef Entity )
{
	BcAssert( Entity->getName().isValid() );
	if( Entity->getParentEntity() == nullptr )
	{
		Entity->setFlag( scnCF_PENDING_DETACH );
		queueComponentForDetach( ScnComponentRef( Entity ) );
	}
	else
	{
		Entity->getParentEntity()->detach( Entity );
	}
}

//////////////////////////////////////////////////////////////////////////
// addCallback
void ScnCore::addCallback( ScnCoreCallback* Callback )
{
	BcAssert( std::find( Callbacks_.begin(), Callbacks_.end(), Callback ) == Callbacks_.end() );
	Callbacks_.push_back( Callback );
}

//////////////////////////////////////////////////////////////////////////
// removeCallback
void ScnCore::removeCallback( ScnCoreCallback* Callback )
{
	auto FoundIt = std::find( Callbacks_.begin(), Callbacks_.end(), Callback );
	BcAssert( FoundIt != Callbacks_.end() );
	Callbacks_.erase( FoundIt );
}

//////////////////////////////////////////////////////////////////////////
// removeAllEntities
void ScnCore::removeAllEntities()
{
	BcU32 ComponentListIdx( ComponentClassIndexMap_[ ScnEntity::StaticGetClass() ] );
	ScnComponentList& ComponentList( ComponentLists_[ ComponentListIdx ] );
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
ScnEntityRef ScnCore::createEntity( const BcName& Package, const BcName& Name, const BcName& InstanceName )
{
	ScnEntityRef Entity;
	ScnEntityRef TemplateEntity;

	// Request template entity.
	if( CsCore::pImpl()->requestResource( Package, Name, TemplateEntity ) )
	{
		BcName UniqueName = Name.getUnique();
		CsPackage* pPackage = CsCore::pImpl()->findPackage( Package );
		BcUnusedVar( pPackage );
#if 0
		Entity = new ScnEntity( TemplateEntity );
		Entity->setName( Name );
		Entity->setOwner( pPackage );
#else
		Entity = ReConstructObject( 
			TemplateEntity->getClass(),
			*Name,
			TemplateEntity->getPackage(),
			TemplateEntity );
#endif
	}

	BcAssertMsg( Entity != nullptr, "ScnCore: Can't create entity \"%s\" from \"%s.%s:%s\"", (*InstanceName).c_str(), (*Package).c_str(), (*Name).c_str(), "ScnEntity" );

	return Entity;	
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
	ScnComponentList& ComponentList( ComponentLists_[ ComponentListIdx ] );
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
	ScnComponentList& ComponentList( ComponentLists_[ ComponentListIdx ] );

	if( Idx < ComponentList.size() )
	{
		return ScnEntityRef( ComponentList[ Idx ] );
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// queueComponentForAttach
void ScnCore::queueComponentForAttach( ScnComponentRef Component )
{
	BcAssert( Component->getName() != BcName::INVALID );
	
	// Add pending operation if not already in queue.
	if( std::find( PendingAttachComponentList_.begin(), PendingAttachComponentList_.end(), Component ) == PendingAttachComponentList_.end() )
	{
		PendingAttachComponentList_.push_back( Component );
	}
}

//////////////////////////////////////////////////////////////////////////
// queueComponentForDetach
void ScnCore::queueComponentForDetach( ScnComponentRef Component )
{
	BcAssert( Component->getName() != BcName::INVALID );
	
	// Add pending operation if not already in queue.
	if( std::find( PendingDetachComponentList_.begin(), PendingDetachComponentList_.end(), Component ) == PendingDetachComponentList_.end() )
	{
		PendingDetachComponentList_.push_back( Component );
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

	// Handle all callbacks.
	std::for_each( Callbacks_.begin(), Callbacks_.end(),
		[ Component ]( ScnCoreCallback* Callback )
		{
			Callback->onAttachComponent( Component );
		} );
	
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
		ScnComponentList& ComponentList( ComponentLists_[ Idx ] );
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

	std::for_each( Callbacks_.begin(), Callbacks_.end(),
		[ Component ]( ScnCoreCallback* Callback )
		{
			Callback->onDetachComponent( Component );
		} );

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
		ScnComponentList& ComponentList( ComponentLists_[ Idx ] );
		ScnComponentListIterator It = std::find( ComponentList.begin(), ComponentList.end(), Component );
		ComponentList.erase( It );
	}
}

//////////////////////////////////////////////////////////////////////////
// processPendingComponents
void ScnCore::processPendingComponents()
{
	while( PendingAttachComponentList_.size() > 0 )
	{
		//
		ScnComponentRef Component( *PendingAttachComponentList_.begin() );
		PendingAttachComponentList_.erase( PendingAttachComponentList_.begin() );

		BcAssert( Component->getInitStage() != CsResource::INIT_STAGE_DESTROY );

		//
		BcAssert( Component->isReady() )

		// Handle attachment.
		if( Component->isFlagSet( scnCF_PENDING_ATTACH ) )
		{
			auto ParentName =
				Component->getParentEntity() ? 
					Component->getParentEntity()->getName() : 
					BcName::INVALID;
			Component->onAttach( Component->getParentEntity() );
			onAttachComponent( ScnEntityWeakRef( Component->getParentEntity() ), ScnComponentRef( Component ) );
			BcAssertMsg( Component->isFlagSet( scnCF_ATTACHED ), "Not attached? Did you call Super::onAttach?" );
		}
	}

	ScnComponentList ComponentToDestroy;
	while( PendingDetachComponentList_.size() > 0 )
	{
		//
		ScnComponentRef Component( *PendingDetachComponentList_.begin() );
		PendingDetachComponentList_.erase( PendingDetachComponentList_.begin() );

		BcAssert( Component->getInitStage() != CsResource::INIT_STAGE_DESTROY );

		//
		BcAssert( Component->isReady() );

		// Handle detachment.
		if( Component->isFlagSet( scnCF_PENDING_DETACH ) )
		{
			auto ParentName =
				Component->getParentEntity() ? 
					Component->getParentEntity()->getName() : 
					BcName::INVALID;
			onDetachComponent( ScnEntityWeakRef( Component->getParentEntity() ), ScnComponentRef( Component ) );
			Component->onDetach( Component->getParentEntity() );
			BcAssertMsg( Component->isFlagSet( scnCF_PENDING_DETACH ) == BcFalse, 
				"Have you called Super::onDetach in type %s?", (*Component->getTypeName()).c_str() );

			// Remove from debug components.
			auto FoundIt = std::find( DebugComponents_.begin(), DebugComponents_.end(), Component );
			if( FoundIt != DebugComponents_.end() )
			{
				DebugComponents_.erase( FoundIt );
			}
		}

		// Handle destruction.
		if( Component->isFlagSet( scnCF_PENDING_DESTROY ) )
		{
			ComponentToDestroy.push_back( Component );
		}
	}

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
	ScnEntity* Entity = createEntity( Params.Package_, Params.Name_, Params.InstanceName_ );

	// Set it's transform.
	Entity->setLocalMatrix( Params.Transform_ );

	// Add entity if we have no parent.
	if( !Params.Parent_.isValid() )
	{
		addEntity( Entity );
	}

	// Visit hierarchy and attach all components.
	Entity->visitHierarchy(
		ScnComponentVisitType::BOTTOM_UP,
		Params.Parent_,
		[ this, &Params ]( ScnComponent* Component, ScnEntity* Parent )
		{
			if( Parent != nullptr )
			{
				Component->setOwner( Parent );
			}

			PSY_LOG( "Component \"%s\" has owner \"%s\"", 
				(*Component->getName()).c_str(),
				(*Component->getRootOwner()->getName()).c_str() );
			
			Component->initialise();
			Component->postInitialise();

			if( Parent != nullptr )
			{
				Parent->attach( Component );
			}
		} );

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
	auto It = EntitySpawnMap_.find( ID );
	BcAssertMsg( It != EntitySpawnMap_.end(), "ScnCore: Spawn ID invalid." );
	ScnEntitySpawnParams& EntitySpawnData( (*It).second );

	// Spawn!
	internalSpawnEntity( EntitySpawnData );

	// Release package, the entity is responsible for it now.
	pPackage->release();

	// Clear out the spawn data.
	EntitySpawnMap_.erase( It );
}

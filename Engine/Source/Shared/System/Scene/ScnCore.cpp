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

#include "System/File/FsCore.h"
#include "System/Os/OsCore.h"
#include "System/Renderer/RsCore.h"

#include "System/Content/CsCore.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "System/Scene/ScnCoreCallback.h"

#include "System/Debug/DsCore.h"

#include "System/Sound/SsCore.h"

#include "System/Scene/ScnViewVisibilityTree.h"
#include "System/Scene/Rendering/ScnLightComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

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
	UpdateEnabled_ = BcTrue;
	StepSingleUpdate_ = BcFalse;
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
	// Look up all component classes and create update lists for them.
	std::vector< ReClass* > ComponentClasses;

	// Extract all the classes with the right attribute.
	auto Classes = ReManager::GetClasses();
	for( auto Class : Classes )
	{
		if( Class->hasBaseClass( ScnComponent::StaticGetClass() ) )
		{
			// Add component class to list for later.
			ComponentClasses.push_back( Class );

			// Find processor.
			auto* Attr = Class->getAttribute< ScnComponentProcessor >();
			if( Attr != nullptr )
			{
				// Initialise.
				Attr->initialise();

				// Get process funcs.
				auto ProcessFuncs = Attr->getProcessFuncs();

				// Add to list.
				for( auto& ProcessFunc : ProcessFuncs )
				{
					ComponentProcessFuncs_.push_back( ProcessFunc );
				}
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
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, this,
		[ this ]( EvtID, const EvtBaseEvent& Event )
		{
			const auto& KeyEvent = Event.get< OsEventInputKeyboard >();
			if( KeyEvent.KeyCode_ == OsEventInputKeyboard::KEYCODE_F5 )
			{
				UpdateEnabled_ = !UpdateEnabled_;
			}
			if( KeyEvent.KeyCode_ == OsEventInputKeyboard::KEYCODE_F6 )
			{
				StepSingleUpdate_ = BcTrue;
			}
			return evtRET_PASS;
		} );

	DsCore::pImpl()->registerPanel(
		"Scene", "Hierarchy", "Ctrl+Shift+H", [ this ]( BcU32 )->void
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
				// TODO: Move these to into a control panel of some kind.
				bool UpdateEnabled = !!UpdateEnabled_;
				if( ImGui::Checkbox( "Enable update (F5)", &UpdateEnabled ) )
				{
					UpdateEnabled_ = UpdateEnabled ? BcTrue : BcFalse;
				}

				if( ImGui::Button( "Step single update (F6)" ) )
				{
					StepSingleUpdate_ = BcTrue;
				}

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
		"Scene", "Component Editor", "Ctrl+Shift+E", [ this ]( BcU32 )->void
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
		"Scene", "Component Process Funcs", "Ctrl+Shift+P", [ this ]( BcU32 )->void
		{
			if ( ImGui::Begin( "Component Process Funcs" ) )
			{
				static bool ShowUnused = false;
				ImGui::Checkbox( "Show unused", &ShowUnused );

				for( auto& ComponentProcessFunc : ComponentProcessFuncs_ )
				{
					auto ComponentListIdx = ComponentClassIndexMap_[ ComponentProcessFunc.Class_ ];
					auto& ComponentList = ComponentLists_[ ComponentListIdx ];
					if( ComponentList.size() > 0 || ShowUnused )
					{
						ImGui::Text( "%s::%s", 
							(*ComponentProcessFunc.Class_->getName()).c_str(),
							ComponentProcessFunc.Name_.c_str() );
						ImGui::Text( "- Priority : %i", 
							ComponentProcessFunc.Priority_ );
						ImGui::Text( "- Components : %u", 
							ComponentList.size() );
					}
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
	PSY_PROFILER_SECTION( UpdateRoot, "ScnCore::update" );

	// Update scene only if we have focus.
	if( OsCore::pImpl()->getClient( 0 ) && OsCore::pImpl()->getClient( 0 )->isActive() )
	{
		auto ShouldUpdateComponents = UpdateEnabled_ || StepSingleUpdate_;

		// Iterate over all component process funcs.
		for( auto& ComponentProcessFunc : ComponentProcessFuncs_ )
		{
			// Always update view, but not other components.
			// TODO: Don't referenece ScnViewComponent in here, use some debug flag later on.
			if( ShouldUpdateComponents || ComponentProcessFunc.Class_ == ScnViewComponent::StaticGetClass() )
			{
				BcLogScopedCategory ScopedCategory( ComponentProcessFunc.Class_->getName() );

				auto ComponentListIdx = ComponentClassIndexMap_[ ComponentProcessFunc.Class_ ];
				auto& ComponentList = ComponentLists_[ ComponentListIdx ];
				ComponentProcessFunc.Func_( ComponentList );
			}
		}

		// Process pending components at the end of the tick.
		// We do this because they can be immediately created,
		// and need a create tick from CsCore next frame.
		processPendingComponents();

		if( StepSingleUpdate_ )
		{
			UpdateEnabled_ = BcFalse;
			StepSingleUpdate_ = BcFalse;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void ScnCore::close()
{
	removeAllEntities();
	processPendingComponents();

	OsCore::pImpl()->unsubscribeAll( this );
	if( SsCore::pImpl() )
	{
		SsCore::pImpl()->stopAllChannels();
	}
	ComponentLists_.clear();

	// Shutdown all component processors.
	for( auto ClassPair : ComponentClassIndexMap_ )
	{
		auto* Attr = ClassPair.first->getAttribute< ScnComponentProcessor >();
		if( Attr != nullptr )
		{
			Attr->shutdown();
		}
	}
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
	if( ( Entity->isFlagSet( scnCF_PENDING_ATTACH ) || Entity->isFlagSet( scnCF_ATTACHED ) ) &&
		!Entity->isFlagSet( scnCF_PENDING_DETACH ) )
	{
		PendingEntityRemovalSet_.insert( Entity );
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
	size_t ComponentListIdx( ComponentClassIndexMap_[ ScnEntity::StaticGetClass() ] );
	ScnComponentList& ComponentList( ComponentLists_[ ComponentListIdx ] );
	for( auto It( ComponentList.rbegin() ); It != ComponentList.rend(); ++It )
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
	PSY_PROFILE_FUNCTION;

	ScnEntityRef Entity;
	ScnEntityRef TemplateEntity;

	// Possibly consider this restriction. Unless the CsCore::requestResource call doesn't guarentee order 
	// (i.e. aliased resources won't be found before the original named one) then we shouldn't need it.
#if 0 

	// Check name does not already exist.
	BcAssertMsg( InstanceName == BcName::INVALID || CsCore::pImpl()->requestResource( Package, InstanceName, TemplateEntity ) == BcFalse, 
		"Entity named %s.%s already exists. Unique name please. Try BcName::getUnique() on name, or BcName::INVALID to automatically assign.", (*Package).c_str(), (*InstanceName).c_str() );
#endif

	// Request template entity.
	if( CsCore::pImpl()->requestResource( Package, Name, TemplateEntity ) )
	{
		CsPackage* pPackage = CsCore::pImpl()->findPackage( Package );
		BcUnusedVar( pPackage );
		Entity = ReConstructObject(
			TemplateEntity->getClass(),
			InstanceName == BcName::INVALID ? *Name.getUnique() : *InstanceName,
			TemplateEntity->getPackage(),
			TemplateEntity );
	}

	BcAssertMsg( Entity != nullptr, "ScnCore: Can't create entity \"%s\" from \"%s.%s:%s\"", (*InstanceName).c_str(), (*Package).c_str(), (*Name).c_str(), "ScnEntity" );

	return Entity;	
}

//////////////////////////////////////////////////////////////////////////
// spawnEntity
ScnEntity* ScnCore::spawnEntity( const ScnEntitySpawnParams& Params )
{
	PSY_PROFILE_FUNCTION;

	BcAssert( BcIsGameThread() );

	// Get package and acquire.
	CsPackage* pPackage = CsCore::pImpl()->requestPackage( Params.Package_ );

	if( pPackage->isReady() )
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
	const auto& ComponentList = getComponentList< ScnEntity >();
	for( auto It( ComponentList.begin() ); It != ComponentList.end(); ++It )
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
	auto& ComponentList = getComponentList( ScnEntity::StaticGetClass() );
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
void ScnCore::visitView( ScnVisitor* pVisitor, const class ScnViewComponent* View )
{
#if 0
	pSpatialTree_->visitView( pVisitor, View );
#else
	// HACK
	auto Idx = ComponentClassIndexMap_[ ScnLightComponent::StaticGetClass() ];
	auto& ComponentList = ComponentLists_[ Idx ];
	for( auto& Component : ComponentList )
	{
		auto* LightComponent = static_cast< ScnLightComponent* >( Component.get() );
		pVisitor->visit( LightComponent );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnCore::visitBounds( class ScnVisitor* pVisitor, const MaAABB& Bounds )
{
#if 0
	pSpatialTree_->visitBounds( pVisitor, Bounds );
#else
	// HACK
	auto Idx = ComponentClassIndexMap_[ ScnLightComponent::StaticGetClass() ];
	auto& ComponentList = ComponentLists_[ Idx ];
	for( auto& Component : ComponentList )
	{
		auto* LightComponent = static_cast< ScnLightComponent* >( Component.get() );
		pVisitor->visit( LightComponent );
	}
#endif
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

	// Call all processor onAttach.
	const ReClass* Class = Component->getClass();
	while( Class->hasBaseClass( ScnComponent::StaticGetClass() ) )
	{
		auto* Attr = Class->getAttribute< ScnComponentProcessor >();
		if( Attr != nullptr )
		{
			Attr->onAttach( Component );
		}
		Class = Class->getSuper();
	}

	// All go into the appropriate list.
	auto& ComponentList = getComponentList( Component->getClass() );
	ComponentList.push_back( Component );
}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
void ScnCore::onDetachComponent( ScnEntityWeakRef Entity, ScnComponent* Component )
{
	// Handle all callbacks.
	std::for_each( Callbacks_.begin(), Callbacks_.end(),
		[ Component ]( ScnCoreCallback* Callback )
		{
			Callback->onDetachComponent( Component );
		} );

	// Call all processor onDetach.
	const ReClass* Class = Component->getClass();
	while( Class->hasBaseClass( ScnComponent::StaticGetClass() ) )
	{
		auto* Attr = Class->getAttribute< ScnComponentProcessor >();
		if( Attr != nullptr )
		{
			Attr->onDetach( Component );
		}
		Class = Class->getSuper();
	}

	// Erase from component list.
	auto& ComponentList = getComponentList( Component->getClass() );
	ScnComponentListIterator It = std::find( ComponentList.begin(), ComponentList.end(), Component );
	ComponentList.erase( It );
}

//////////////////////////////////////////////////////////////////////////
// processPendingComponents
void ScnCore::processPendingComponents()
{
	for( auto Entity : PendingEntityRemovalSet_ )
	{
		if( Entity->getParentEntity() == nullptr )
		{
			Entity->detachAll();
			Entity->setFlag( scnCF_PENDING_DETACH );
			queueComponentForDetach( ScnComponentRef( Entity ) );
		}
		else
		{
			Entity->getParentEntity()->detach( Entity );
		}
	}
	PendingEntityRemovalSet_.clear();

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
	PSY_PROFILE_FUNCTION;

	// Create entity.
	ScnEntity* Entity = createEntity( Params.Package_, Params.Name_, Params.InstanceName_ );

	if( Entity == nullptr )
	{
		PSY_LOG( "ERROR: Failed to spawn entity %s.%s.",
			(*Params.Package_).c_str(), 
			(*Params.Name_).c_str() );
	}
	// Set it's transform.
	Entity->setLocalMatrix( Params.Transform_ );

	// Initialise entity.
	Entity->initialise();
	Entity->postInitialise();

	// Add entity if we have no parent.
	if( !Params.Parent_.isValid() )
	{
		addEntity( Entity );
	}

	// Visit hierarchy and attach all components.
	{
		PSY_PROFILER_SECTION( Root, "Visit hierarchy" );
		Entity->visitHierarchy(
			ScnComponentVisitType::BOTTOM_UP,
			Params.Parent_,
			[ this, &Params, Entity ]( ScnComponent* Component, ScnEntity* Parent )
			{
				// If component doesn't have a basis, then it has been added
				// during this visit. We can skip it.
				if( Component->getBasis() )
				{
					PSY_LOG( "Component \"%s\" has package \"%s\"", 
						(*Component->getName()).c_str(),
						(*Component->getPackage()->getName()).c_str() );
					if( Parent != nullptr )
					{
						PSY_LOG( "Component's parent \"%s\" has package \"%s\"", 
							(*Parent->getName()).c_str(),
							(*Parent->getPackage()->getName()).c_str() );
					}
			
					if( Parent != nullptr && Component != Entity )
					{
						Component->initialise();
						Component->postInitialise();
						Component->setOwner( Parent );
					}

					if( Parent != nullptr )
					{
						Parent->attach( Component );
					}
				}
			} );
	}

	// Call on spawn callback.
	if( Params.OnSpawn_ != nullptr )
	{
		PSY_PROFILER_SECTION( Root, "On spawn" );
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

//////////////////////////////////////////////////////////////////////////
// getComponentList 
ScnComponentList& ScnCore::getComponentList( const ReClass* Class )
{
	auto FoundIt = ComponentClassIndexMap_.find( Class );
	BcAssert( FoundIt != ComponentClassIndexMap_.end() );
	return ComponentLists_[ FoundIt->second ];
}

//////////////////////////////////////////////////////////////////////////
// getComponentList 
const ScnComponentList& ScnCore::getComponentList( const ReClass* Class ) const
{
	auto FoundIt = ComponentClassIndexMap_.find( Class );
	BcAssert( FoundIt != ComponentClassIndexMap_.end() );
	return ComponentLists_[ FoundIt->second ];
}

/**************************************************************************
*
* File:		ScnEntity.cpp
* Author:	Neil Richardson 
* Ver/Date:	26/11/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnEntity.h"
#include "System/Scene/ScnCore.h"
#include "System/Content/CsCore.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "Base/BcProfiler.h"

#include "System/Content/CsCore.h"
#include "System/Renderer/RsCore.h"
#include "Events/EvtProxyBuffered.h"

#include "Serialisation/SeJsonWriter.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnEntityImport.h"
#endif

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnEntity );

void ScnEntity::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "LocalTransform_",	&ScnEntity::LocalTransform_, bcRFF_IMPORTER ),
		new ReField( "WorldTransform_",	&ScnEntity::WorldTransform_ ),
		new ReField( "Components_",		&ScnEntity::Components_, bcRFF_OWNER ),
#if SCNENTITY_USES_EVTPUBLISHER
		new ReField( "pEventProxy_",	&ScnEntity::pEventProxy_, bcRFF_TRANSIENT ),
#endif
	};
	
	using namespace std::placeholders;
	auto& Class = ReRegisterClass< ScnEntity, Super >( Fields );
	Class.addAttribute( new ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Update",
				ScnComponentPriority::ENTITY_UPDATE,
				std::bind( &ScnEntity::update, _1 ) ),
		} ) );
#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnEntityImport::StaticGetClass(), 0, 200 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntity::ScnEntity():
	pHeader_( nullptr )
#if SCNENTITY_USES_EVTPUBLISHER
	, pEventProxy_( nullptr )
#endif
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEntity::ScnEntity( ReNoInit ):
	pHeader_( nullptr )
#if SCNENTITY_USES_EVTPUBLISHER
	, pEventProxy_( nullptr )
#endif
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnEntity::~ScnEntity()
{
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise()
{
	// If we have a basis, we need to acquire its package.
	if( getBasis() != nullptr )
	{
		getPackage()->acquire();
	}
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ScnEntity::destroy()
{
	// If we have a basis entity, we need to release the package.
	if( getBasis() != nullptr )
	{
		getPackage()->release();
	}
}

//////////////////////////////////////////////////////////////////////////
// visitHierarchy
void ScnEntity::visitHierarchy( 
		ScnComponentVisitType VisitType, 
		ScnEntity* Parent,
		const ScnComponentVisitFunc& Func )
{
	if( VisitType == ScnComponentVisitType::BOTTOM_UP )
	{
		Super::visitHierarchy( VisitType, Parent, Func );
	}

	for( size_t Idx = 0; Idx < Components_.size(); ++Idx )
	{
		auto Component = Components_[ Idx ];
		Component->visitHierarchy( VisitType, this, Func );
	}

	if( VisitType == ScnComponentVisitType::TOP_DOWN )
	{
		Super::visitHierarchy( VisitType, Parent, Func );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttachScene
void ScnEntity::onAttach( ScnEntityWeakRef Parent )
{
#if SCNENTITY_USES_EVTPUBLISHER
	// Setup buffered event proxy.
	pEventProxy_ = new EvtProxyBuffered( this );	
#endif

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onAttachScene
void ScnEntity::onDetach( ScnEntityWeakRef Parent )
{
#if SCNENTITY_USES_EVTPUBLISHER
	// Free event proxy.
	delete pEventProxy_;
	pEventProxy_ = nullptr;
#endif

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// attach
void ScnEntity::attach( ScnComponent* Component )
{
	BcAssert( Component != nullptr );
	BcAssert( Component->getName() != BcName::INVALID );
	BcAssertMsg( Component != nullptr, "Trying to attach a null component!" );
	BcAssertMsg( Component->isFlagSet( scnCF_ANY ) == BcFalse, "Entity pending operations already." );
	ScnComponentListIterator It = std::find( Components_.begin(), Components_.end(), Component );
	if( It == Components_.end() )
	{
		Components_.push_back( Component );	
	}
	Component->setParentEntity( ScnEntityWeakRef( this ) );
	Component->setFlag( scnCF_PENDING_ATTACH );
	ScnCore::pImpl()->queueComponentForAttach( Component );
}

//////////////////////////////////////////////////////////////////////////
// detach
void ScnEntity::detach( ScnComponent* Component )
{
	if( ( Component->isFlagSet( scnCF_ATTACHED ) || Component->isFlagSet( scnCF_PENDING_ATTACH ) ) &&
		!Component->isFlagSet( scnCF_PENDING_DETACH ) )
	{
		
		// If component is an entity, recurse down and detach everything first.
		if( Component->isTypeOf< ScnEntity >() )
		{
			ScnEntityRef Entity = Component;
			Entity->detachAll();
		}

		BcAssert( Component->getName() != BcName::INVALID );
		ScnComponentListIterator It = std::find( Components_.begin(), Components_.end(), Component );
		BcAssert( It != Components_.end() );
		if( It != Components_.end() )
		{
			Components_.erase( It );	
		}
		BcAssertMsg( Component != nullptr, "Trying to detach a null component!" );
		Component->setFlag( scnCF_PENDING_DETACH );
		ScnCore::pImpl()->queueComponentForDetach( Component );
	}
}

//////////////////////////////////////////////////////////////////////////
// detachAll
void ScnEntity::detachAll()
{
	// Recurse entity and detach every component reverse order & top->bottom.
	visitHierarchy( ScnComponentVisitType::TOP_DOWN, this,
		[ this ]( ScnComponent* Component, ScnEntity* Parent )
		{
			if( Component->isTypeOf< ScnEntity >() )
			{
				// Remove components.
				ScnEntity* Entity = static_cast< ScnEntity* >( Component );

				for( size_t Idx = Entity->getNoofComponents(); Idx > 0; --Idx )
				{
					auto EntityComponent = Entity->getComponent( Idx - 1 );
					Entity->detach( EntityComponent );
				}
			}
		});
}

//////////////////////////////////////////////////////////////////////////
// getBasisEntity
ScnEntityRef ScnEntity::getBasisEntity()
{
	// If we have a basis, ask it for it's basis.
	auto Basis = static_cast< ScnEntity* >( getBasis() );
	if( Basis != nullptr )
	{
		return Basis->getBasisEntity();
	}

	// We have no basis, therefore we are it.
	return this;
}

//////////////////////////////////////////////////////////////////////////
// getNoofComponents
size_t ScnEntity::getNoofComponents() const
{
	return Components_.size();
}
	
//////////////////////////////////////////////////////////////////////////
// getComponent
const ScnComponentList& ScnEntity::getComponents() const
{
	return Components_;
}

//////////////////////////////////////////////////////////////////////////
// getComponent
ScnComponent* ScnEntity::getComponent( size_t Idx, const ReClass* Class )
{
	PSY_PROFILE_FUNCTION;

	if( Class == nullptr )
	{
		size_t CurrIdx = 0;
		for( size_t ComponentIdx = 0; ComponentIdx < Components_.size(); ++ComponentIdx )
		{
			if( CurrIdx++ == Idx )
			{
				return Components_[ ComponentIdx ];
			}
		}
	}
	else
	{
		size_t NoofComponents = getNoofComponents();
		size_t SearchIdx = 0;
		for( size_t ComponentIdx = 0; ComponentIdx < NoofComponents; ++ComponentIdx )
		{
			ScnComponentRef Component = getComponent( ComponentIdx );
			if( Component->getClass()->hasBaseClass( Class ) )
			{
				if( SearchIdx == Idx )
				{
					return Component;
				}

				++SearchIdx;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getComponent
ScnComponent* ScnEntity::getComponent( BcName Name, const ReClass* Class )
{
	PSY_PROFILE_FUNCTION;

	if( Class == nullptr )
	{
		for( BcU32 ComponentIdx = 0; ComponentIdx < Components_.size(); ++ComponentIdx )
		{
			if( Components_[ ComponentIdx ]->getName() == Name )
			{
				return Components_[ ComponentIdx ];
			}
		}
	}
	else
	{
		size_t NoofComponents = getNoofComponents();
		for( size_t ComponentIdx = 0; ComponentIdx < NoofComponents; ++ComponentIdx )
		{
			ScnComponentRef Component = getComponent( ComponentIdx );
			if( Component->getName() == Name && Component->getClass()->hasBaseClass( Class ) )
			{
				return Component;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getComponentAnyParent
ScnComponent* ScnEntity::getComponentAnyParent( size_t Idx, const ReClass* Class )
{
	PSY_PROFILE_FUNCTION;

	ScnComponentRef Component = getComponent( Idx, Class );

	if( Component.isValid() == BcFalse && getParentEntity() != NULL )
	{
		Component = getParentEntity()->getComponentAnyParent( Idx, Class );
	}

	return Component;
}

//////////////////////////////////////////////////////////////////////////
// getComponentAnyParent
ScnComponent* ScnEntity::getComponentAnyParent( BcName Name, const ReClass* Class )
{
	PSY_PROFILE_FUNCTION;

	ScnComponentRef Component = getComponent( Name, Class );

	if( Component.isValid() == BcFalse && getParentEntity() != NULL )
	{
		Component = getParentEntity()->getComponentAnyParent( Name, Class );
	}

	return Component;
}

//////////////////////////////////////////////////////////////////////////
// setLocalPosition
void ScnEntity::setLocalPosition( const MaVec3d& Position )
{
	LocalTransform_.translation( Position );
}

//////////////////////////////////////////////////////////////////////////
// setLocalMatrix
void ScnEntity::setLocalMatrix( const MaMat4d& Matrix )
{
	LocalTransform_ = Matrix;
	if( ParentEntity_ == nullptr )
	{
		WorldTransform_ = Matrix;
	}
}

//////////////////////////////////////////////////////////////////////////
// setLocalMatrixRS
void ScnEntity::setLocalMatrixRS( const MaMat4d& Matrix )
{
	LocalTransform_.row0( Matrix.row0() );
	LocalTransform_.row1( Matrix.row1() );
	LocalTransform_.row2( Matrix.row2() );
	if( ParentEntity_ == nullptr )
	{
		WorldTransform_ = Matrix;
		WorldTransform_.row0( Matrix.row0() );
		WorldTransform_.row1( Matrix.row1() );
		WorldTransform_.row2( Matrix.row2() );
	}
}

//////////////////////////////////////////////////////////////////////////
// getLocalPosition
MaVec3d ScnEntity::getLocalPosition() const
{
	return LocalTransform_.translation();
}

//////////////////////////////////////////////////////////////////////////
// setWorldPosition
void ScnEntity::setWorldPosition( const MaVec3d& Position )
{
	MaMat4d InverseParentWorldTransform = 
		ParentEntity_ != nullptr ? 
			ParentEntity_->WorldTransform_ : MaMat4d();
	InverseParentWorldTransform.inverse();

	WorldTransform_.translation( Position );
	setLocalMatrix( InverseParentWorldTransform * WorldTransform_ );
}

//////////////////////////////////////////////////////////////////////////
// setWorldMatrix
void ScnEntity::setWorldMatrix( const MaMat4d& Matrix )
{
	MaMat4d InverseParentWorldTransform =
		ParentEntity_ != nullptr ?
			ParentEntity_->WorldTransform_ : MaMat4d();
	InverseParentWorldTransform.inverse();

	WorldTransform_ = Matrix;
	setLocalMatrix( InverseParentWorldTransform * Matrix );
}

//////////////////////////////////////////////////////////////////////////
// setWorldMatrixRS
void ScnEntity::setWorldMatrixRS( const MaMat4d& Matrix )
{
	MaMat4d InverseParentWorldTransform =
		ParentEntity_ != nullptr ?
		ParentEntity_->WorldTransform_ : MaMat4d();
	InverseParentWorldTransform.inverse();

	WorldTransform_ = Matrix;
	setLocalMatrixRS( InverseParentWorldTransform * Matrix );
}

//////////////////////////////////////////////////////////////////////////
// getWorldPosition
MaVec3d ScnEntity::getWorldPosition() const
{
	return WorldTransform_.translation();
}

//////////////////////////////////////////////////////////////////////////
// getLocalMatrix
const MaMat4d& ScnEntity::getLocalMatrix() const
{
	return LocalTransform_;
}

//////////////////////////////////////////////////////////////////////////
// getWorldMatrix
const MaMat4d& ScnEntity::getWorldMatrix() const
{
	return WorldTransform_;
}

//////////////////////////////////////////////////////////////////////////
// internalCreateComponent
ScnComponent* ScnEntity::internalCreateComponent( const BcName& Name, const ReClass* Class )
{
	BcAssert( Class != nullptr );
	ReObjectRef< CsResource > Resource;
	auto RetVal = CsCore::pImpl()->internalCreateResource( 
		Name, Class, BcErrorCode, getPackage(), Resource );
	BcAssertMsg( RetVal, "Unable to create component for \"%s\"", (*Class->getName()).c_str() );
	BcUnusedVar( RetVal );
	auto Component = ReObjectRef< ScnComponent >( Resource );
	BcAssert( Component.isValid() );
	return Component;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnEntity::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnEntity::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = reinterpret_cast< const ScnEntityHeader* >( pData );

		LocalTransform_ = pHeader_->LocalTransform_;

		setupComponents();

		CsResource::markCreate();
		CsResource::markReady();
	}
}

//////////////////////////////////////////////////////////////////////////
// setupComponents
void ScnEntity::setupComponents()
{
	// New stuff.
	if( pHeader_ != nullptr && Components_.size() == 0 )
	{
		auto Basis = static_cast< ScnEntity* >( getBasis() );
		const ScnEntityHeader* Header = pHeader_ == nullptr ? Basis->pHeader_ : pHeader_;
		const BcU32* ComponentCrossRefs = reinterpret_cast< const BcU32* >( Header + 1 );
		for( BcU32 Idx = 0; Idx < Header->NoofComponents_; ++Idx )
		{
			// We are a basis.
			if( getBasis() == nullptr )
			{
				ScnComponentRef Component = getPackage()->getCrossRefResource( ComponentCrossRefs[ Idx ] );
				BcAssertMsg( Component, "A component is null in ScnEntity \"%s\". Is type valid to use as component?", getFullName().c_str() );
				Components_.push_back( Component );
			}
			else
			{
				BcAssert( Basis->Components_.size() == Header->NoofComponents_ );
				ScnComponentRef Component = Basis->Components_[ Idx ];

				// Construct a new entity.
				ScnComponentRef NewComponent = 
					ReConstructObject( 
						Component->getClass(), 
						*Component->getName().getUnique(), 
						getPackage(), 
						Component );

				NewComponent->visitHierarchy(
					ScnComponentVisitType::BOTTOM_UP,
					this,
					[ this ]( ScnComponent* Component, ScnEntity* Parent )
					{
						Component->setOwner( getPackage() );
						Component->initialise();
						Parent->attach( Component );
					} );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//static
void ScnEntity::update( const ScnComponentList& Components )
{
	PSY_PROFILER_SECTION( UpdateRoot, "ScnEntity::update" );

	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnEntity >() );
		auto* Entity = static_cast< ScnEntity* >( Component.get() );

		BcAssert( Entity->getPackage() != nullptr );
		
		// Get parent entity to calculate world transform.
		if( Entity->getParentEntity() )
		{
			Entity->WorldTransform_ = Entity->LocalTransform_ * Entity->getParentEntity()->WorldTransform_;
		}
		else
		{
			Entity->WorldTransform_ = Entity->LocalTransform_;
		}
		
#if SCNENTITY_USES_EVTPUBLISHER
		// Dispatch all events.
		// TODO: Move outside of update, perhaps run before all other component updates?
		Entity->pEventProxy_->dispatch();
#endif
	}
}

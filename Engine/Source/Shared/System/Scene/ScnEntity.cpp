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

#include "Base/BcProfiler.h"

#include "System/Renderer/RsCore.h"
#include "Events/EvtProxyBuffered.h"

#include "Serialisation/SeJsonWriter.h"

#ifdef PSY_SERVER
#include "System/Scene/Import/ScnEntityImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnEntity );

void ScnEntity::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Basis_",			&ScnEntity::Basis_, bcRFF_SHALLOW_COPY ),
		new ReField( "LocalTransform_",	&ScnEntity::LocalTransform_ ),
		new ReField( "WorldTransform_",	&ScnEntity::WorldTransform_ ),
		new ReField( "Components_",		&ScnEntity::Components_, bcRFF_TRANSIENT ),
		new ReField( "pEventProxy_",	&ScnEntity::pEventProxy_, bcRFF_TRANSIENT ),
	};
		
	auto& Class = ReRegisterClass< ScnEntity, Super >( Fields );
		
	Class.addAttribute( new ScnComponentAttribute( -2100 ) );
#ifdef PSY_SERVER
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( ScnEntityImport::StaticGetClass() ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise()
{
	Super::initialise();

	// NULL internals.
	pHeader_ = nullptr;
	pEventProxy_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise( ScnEntityRef Basis )
{
	ScnEntity::initialise();

	// Grab our basis.
	Basis_ = Basis->getBasisEntity();
	pHeader_ = Basis->pHeader_;

	BcAssertMsg( Basis_->isReady(), "Basis entity is not ready!" );

	// Copy over internals.
	LocalTransform_ = Basis_->LocalTransform_;

	// Acquire basis package.
	Basis_->getPackage()->acquire();
}

//////////////////////////////////////////////////////////////////////////
// create
void ScnEntity::create()
{
	// New stuff.
	const ScnEntityHeader* Header = pHeader_ == nullptr ? Basis_->pHeader_ : pHeader_;
	const BcU32* ComponentCrossRefs = reinterpret_cast< const BcU32* >( Header + 1 );

	for( BcU32 Idx = 0; Idx < Header->NoofComponents_; ++Idx )
	{
		// We are a basis.
		if( Basis_ == nullptr )
		{
			ScnComponentRef Component = getPackage()->getPackageCrossRef( ComponentCrossRefs[ Idx ] );
			Components_.push_back( Component );
		}
		else
		{
			BcAssert( Basis_->Components_.size() == Header->NoofComponents_ );
			ScnComponentRef Component = Basis_->Components_[ Idx ];

			if( Component->getName() == "XREF13ScnFontComponent_0" )
			{
				int a=  0; ++a;
			}

			// Construct a new entity.
			ScnComponentRef NewComponent = 
				ReConstructObject( Component->getClass(), *Component->getName().getUnique(), getPackage(), Component );

			attach( NewComponent );
		}
	}
	
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ScnEntity::destroy()
{
	// If we have a basis entity, we need to release the package.
	if( Basis_.isValid() )
	{
		getPackage()->release();
	}
}

//////////////////////////////////////////////////////////////////////////
// update
void ScnEntity::update( BcF32 Tick )
{
	PSY_PROFILER_SECTION( UpdateRoot, std::string( "ScnEntity::update (" ) + *getName() + std::string( ")" ) );

	BcAssert( getPackage() != NULL );
	
	// Get parent entity to calculate world transform.
	if( getParentEntity() )
	{
		WorldTransform_ = LocalTransform_ * getParentEntity()->WorldTransform_;
	}
	else
	{
		WorldTransform_ = LocalTransform_;
	}
	
	// Update as component first.
	Super::update( Tick );

	// Dispatch all events.
	pEventProxy_->dispatch();
}

//////////////////////////////////////////////////////////////////////////
// attach
void ScnEntity::attach( ScnComponent* Component )
{
	BcAssert( Component->getName() != BcName::INVALID );
	ScnComponentListIterator It = std::find( Components_.begin(), Components_.end(), Component );
	if( It == Components_.end() )
	{
		BcAssertMsg( Component != NULL, "Trying to attach a null component!" );

		Component->setParentEntity( ScnEntityWeakRef( this ) );
		Component->setFlag( scnCF_PENDING_ATTACH );
		Components_.push_back( Component );
	
		ScnCore::pImpl()->queueComponentAsPendingOperation( Component );
	}
}

//////////////////////////////////////////////////////////////////////////
// detach
void ScnEntity::detach( ScnComponent* Component )
{
	BcAssert( Component->getName() != BcName::INVALID );
	ScnComponentListIterator It = std::find( Components_.begin(), Components_.end(), Component );
	if( It != Components_.end() )
	{
		BcAssertMsg( Component != NULL, "Trying to detach a null component!" );
		BcAssertMsg( !Component->isFlagSet( scnCF_PENDING_ATTACH ), 
			"Component is currently pending attachment! Being attached too quickly?" )

		Component->setFlag( scnCF_PENDING_DETACH );
		Components_.erase( It );

		ScnCore::pImpl()->queueComponentAsPendingOperation( Component );
	}
}

//////////////////////////////////////////////////////////////////////////
// detachFromParent
void ScnEntity::detachFromParent()
{
	BcAssertMsg( getParentEntity() == NULL, "Can't detach entity \"%s\", it's not attached.", (*getName()).c_str() );
	getParentEntity()->detach( this );
}

//////////////////////////////////////////////////////////////////////////
// onAttachScene
void ScnEntity::onAttach( ScnEntityWeakRef Parent )
{
	// Setup buffered event proxy.
	pEventProxy_ = new EvtProxyBuffered( this );	

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onAttachScene
void ScnEntity::onDetach( ScnEntityWeakRef Parent )
{
	// Free event proxy.
	delete pEventProxy_;
	pEventProxy_ = NULL;

	// All our child components want to detach.
	while( Components_.size() > 0 )
	{
		ScnComponentRef Component( *Components_.begin() );
		detach( Component );
	}

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getBasisEntity
ScnEntityRef ScnEntity::getBasisEntity()
{
	// If we have a basis, ask it for it's basis.
	if( Basis_.isValid() )
	{
		return Basis_->getBasisEntity();
	}

	// We have no basis, therefore we are it.
	return this;
}

//////////////////////////////////////////////////////////////////////////
// getNoofComponents
BcU32 ScnEntity::getNoofComponents() const
{
	return Components_.size();
}
	
//////////////////////////////////////////////////////////////////////////
// getComponent
ScnComponent* ScnEntity::getComponent( BcU32 Idx, const ReClass* Class )
{
	if( Class == nullptr )
	{
		BcU32 CurrIdx = 0;
		for( BcU32 ComponentIdx = 0; ComponentIdx < Components_.size(); ++ComponentIdx )
		{
			if( CurrIdx++ == Idx )
			{
				return Components_[ ComponentIdx ];
			}
		}
	}
	else
	{
		BcU32 NoofComponents = getNoofComponents();
		BcU32 SearchIdx = 0;
		for( BcU32 ComponentIdx = 0; ComponentIdx < NoofComponents; ++ComponentIdx )
		{
			ScnComponentRef Component = getComponent( ComponentIdx );
			if( Component->getClass() == Class )
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
		BcU32 NoofComponents = getNoofComponents();
		BcU32 SearchIdx = 0;
		for( BcU32 ComponentIdx = 0; ComponentIdx < NoofComponents; ++ComponentIdx )
		{
			ScnComponentRef Component = getComponent( ComponentIdx );
			if( Component->getName() == Name && Component->getClass() == Class )
			{
				return Component;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getComponentAnyParent
ScnComponent* ScnEntity::getComponentAnyParent( BcU32 Idx, const ReClass* Class )
{
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
// getLocalPosition
MaVec3d ScnEntity::getLocalPosition() const
{
	return LocalTransform_.translation();
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
		pJsonObject_ = nullptr;
		pHeader_ = reinterpret_cast< const ScnEntityHeader* >( pData );
		CsResource::markCreate();
	}
}

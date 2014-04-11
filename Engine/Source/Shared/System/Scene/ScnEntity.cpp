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

#ifdef PSY_SERVER
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnEntity::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	// Write out object to be used later.
	Json::FastWriter Writer;
	std::string JsonData = Writer.write( Object );
	
	//
	Importer.addChunk( BcHash( "object" ), JsonData.c_str(), JsonData.size() + 1 );
	
	//
	return Super::import( Importer, Object );
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnEntity );

BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnEntity )
	BCREFLECTION_MEMBER( ScnEntity,							Basis_,							bcRFF_REFERENCE | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcMat4d,							LocalTransform_,				bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcMat4d,							WorldTransform_,				bcRFF_DEFAULT ),
BCREFLECTION_DERIVED_END();

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise()
{
	Super::initialise();

	pEventProxy_ = NULL;

	// NULL internals.
	pJsonObject_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise( ScnEntityRef Basis )
{
	ScnEntity::initialise();

	// Grab our basis.
	Basis_ = Basis->getBasisEntity();

	BcAssertMsg( Basis_->isReady(), "Basis entity is not ready!" );

	// Copy over internals.
	pJsonObject_ = Basis_->pJsonObject_;
	LocalTransform_ = Basis_->LocalTransform_;

	// Create components from Json.
	// TEMP.
	Json::Value Root;
	Json::Reader Reader;
	if( Reader.parse( pJsonObject_, Root ) )
	{
		//
		const Json::Value& Components = Root[ "components" ];

		for( BcU32 Idx = 0; Idx < Components.size(); ++Idx )
		{
			const Json::Value& Component( Components[ Idx ] );
			CsResourceRef<> ResourceRef;
			BcName Type = Component[ "type" ].asCString();
			const Json::Value& NameValue = Component[ "name" ];

			BcName Name = NameValue.type() == Json::stringValue ? NameValue.asCString() : BcName::INVALID;

			if( CsCore::pImpl()->internalCreateResource( Name, Type, BcErrorCode, getPackage(), ResourceRef ) )
			{
				ScnComponentRef ComponentRef( ResourceRef );
				BcAssert( ComponentRef.isValid() );

				//BcPrintf( "** - %s:%s\n", (*ComponentRef->getName()).c_str(), (*ComponentRef->getType()).c_str() );

				// Initialise has already been called...need to change this later.
				ComponentRef->initialise( Component );

				//ComponentRef->serialiseProperties();

				// Attach.
				attach( ComponentRef );
			}			
		}
	}

	// If we have a basis entity, we need to acquire the package.
	if( Basis_.isValid() )
	{
		BcAssertMsg( Basis_->getPackage() == getPackage(), "Must reference the same package as the basis entity." );
		getPackage()->acquire();
	}

	// Placeholder!
	//serialiseProperties();
}

//////////////////////////////////////////////////////////////////////////
// create
void ScnEntity::create()
{
	// If we have a basis, we're ready.
	if( Basis_.isValid() )
	{			
		markReady();
	}
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
	ScnComponentListIterator It = std::find( Components_.begin(), Components_.end(), Component );
	if( It != Components_.end() )
	{
		BcAssertMsg( Component != NULL, "Trying to detach a null component!" );
		BcAssertMsg( !Component->isFlagSet( scnCF_PENDING_ATTACH ), "Component is currently pending attachment!" )

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
ScnComponent* ScnEntity::getComponent( BcU32 Idx, const BcName& Type )
{
	if( Type == BcName::INVALID )
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
			if( Component->getTypeName() == Type )
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
ScnComponent* ScnEntity::getComponent( BcName Name, const BcName& Type )
{
	if( Type == BcName::INVALID )
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
			if( Component->getName() == Name && Component->getTypeName() == Type )
			{
				return Component;
			}
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getComponentAnyParent
ScnComponent* ScnEntity::getComponentAnyParent( BcU32 Idx, const BcName& Type )
{
	ScnComponentRef Component = getComponent( Idx, Type );

	if( Component.isValid() == BcFalse && getParentEntity() != NULL )
	{
		Component = getParentEntity()->getComponentAnyParent( Idx, Type );
	}

	return Component;
}

//////////////////////////////////////////////////////////////////////////
// getComponentAnyParent
ScnComponent* ScnEntity::getComponentAnyParent( BcName Name, const BcName& Type )
{
	ScnComponentRef Component = getComponent( Name, Type );

	if( Component.isValid() == BcFalse && getParentEntity() != NULL )
	{
		Component = getParentEntity()->getComponentAnyParent( Name, Type );
	}

	return Component;
}

//////////////////////////////////////////////////////////////////////////
// setLocalPosition
void ScnEntity::setLocalPosition( const BcVec3d& Position )
{
	LocalTransform_.translation( Position );
}

//////////////////////////////////////////////////////////////////////////
// setLocalMatrix
void ScnEntity::setLocalMatrix( const BcMat4d& Matrix )
{
	LocalTransform_ = Matrix;
}

//////////////////////////////////////////////////////////////////////////
// getLocalPosition
BcVec3d ScnEntity::getLocalPosition() const
{
	return LocalTransform_.translation();
}

//////////////////////////////////////////////////////////////////////////
// getWorldPosition
BcVec3d ScnEntity::getWorldPosition() const
{
	return WorldTransform_.translation();
}

//////////////////////////////////////////////////////////////////////////
// getLocalMatrix
const BcMat4d& ScnEntity::getLocalMatrix() const
{
	return LocalTransform_;
}

//////////////////////////////////////////////////////////////////////////
// getWorldMatrix
const BcMat4d& ScnEntity::getWorldMatrix() const
{
	return WorldTransform_;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnEntity::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );

	// As we are being loaded by a package, we don't need to reference it.
	// We acquire in the init of ScnComponent for the general case,
	// but we can release 
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnEntity::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "object" ) )
	{
		pJsonObject_ = reinterpret_cast< const BcChar* >( pData );

		CsResource::markReady();
	}
}

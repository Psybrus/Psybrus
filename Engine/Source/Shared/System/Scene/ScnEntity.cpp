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

void ScnEntity::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Basis_",			&ScnEntity::Basis_ ),
		ReField( "LocalTransform_",	&ScnEntity::LocalTransform_ ),
		ReField( "WorldTransform_",	&ScnEntity::WorldTransform_ ),
		ReField( "Components_",		&ScnEntity::Components_ ),
		ReField( "pEventProxy_",	&ScnEntity::pEventProxy_ ),
	};
		
	ReRegisterClass< ScnEntity, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2100 ) );
}

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
			ReObjectRef< CsResource > ResourceRef;
			const ReClass* Class = ReManager::GetClass( Component[ "type" ].asCString() );
			const Json::Value& NameValue = Component[ "name" ];

			BcName Name = NameValue.type() == Json::stringValue ? NameValue.asCString() : BcName::INVALID;

			if( CsCore::pImpl()->internalCreateResource( Name, Class, BcErrorCode, getPackage(), ResourceRef ) )
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

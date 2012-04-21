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

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise()
{
	// NULL internals.
	IsAttached_ = BcFalse;
	pJsonObject_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise( ScnEntityRef Basis )
{
	// Grab our basis.
	Basis_ = Basis->getBasisEntity();

	// Copy over internals.
	IsAttached_ = BcFalse;
	pJsonObject_ = Basis_->pJsonObject_;
	Transform_ = Basis_->Transform_;

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
			if( CsCore::pImpl()->internalCreateResource( BcName::INVALID, Component[ "type" ].asCString(), BcErrorCode, NULL, ResourceRef ) )
			{
				ScnComponentRef ComponentRef( ResourceRef );
				BcAssert( ComponentRef.isValid() );

				// Initialise has already been called...need to change this later.
				ComponentRef->initialise( Component );

				// Attach.
				attach( ComponentRef );
			}			
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// create
void ScnEntity::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
void ScnEntity::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// isReady
BcBool ScnEntity::isReady()
{
	// TODO: Set a flag internally once stuff has loaded. Will I ever fucking get round to this!?
	for( ScnComponentListIterator It( Components_.begin() ); It != Components_.end(); ++It )
	{
		ScnComponentRef& Component( *It );

		if( Component.isReady() == BcFalse )
		{
			return BcFalse;
		}
	}

	return pJsonObject_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
void ScnEntity::update( BcReal Tick )
{
	// Update as component first.
	Super::update( Tick );

	// Process attach/detach.
	processAttachDetach();

	// Update components.
	for( ScnComponentListIterator It( Components_.begin() ); It != Components_.end(); ++It )
	{
		ScnComponentRef& Component( *It );

		Component->update( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnEntity::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	// Set all material parameters that the view has info on. (HACK).
	for( ScnComponentListIterator It( Components_.begin() ); It != Components_.end(); ++It )
	{
		ScnMaterialComponentRef MaterialComponent( *It );
		if( MaterialComponent.isValid() )
		{
			pViewComponent->setMaterialParameters( MaterialComponent );
		}
	}

	// Render all renderable components.
	for( ScnComponentListIterator It( Components_.begin() ); It != Components_.end(); ++It )
	{
		ScnRenderableComponentWeakRef RenderableComponent( *It );
		if( RenderableComponent.isValid() )
		{
			RenderableComponent->render( pViewComponent, pFrame, Sort );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// attach
void ScnEntity::attach( ScnComponent* Component )
{
	BcAssertMsg( Component != NULL, "Trying to attach a null component!" );

	AttachComponents_.push_back( Component );
}

//////////////////////////////////////////////////////////////////////////
// detach
void ScnEntity::detach( ScnComponent* Component )
{
	BcAssertMsg( Component != NULL, "Trying to detach a null component!" );

	DetachComponents_.push_back( Component );
}

//////////////////////////////////////////////////////////////////////////
// reattach
void ScnEntity::reattach( ScnComponent* Component )
{
	detach( Component );
	attach( Component );
}

//////////////////////////////////////////////////////////////////////////
// onAttachScene
void ScnEntity::onAttachScene()
{
	// Process attach/detach.
	processAttachDetach();

	BcAssert( IsAttached_ == BcFalse );
	IsAttached_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// onDetachScene
void ScnEntity::onDetachScene()
{
	BcAssert( IsAttached_ == BcTrue );
	IsAttached_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// isAttached
BcBool ScnEntity::isAttached() const
{
	return IsAttached_ || Super::isAttached();
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
	// HACK.
	return Components_.size() > 0 ? Components_.size() : AttachComponents_.size();
}
	
//////////////////////////////////////////////////////////////////////////
// getComponent
ScnComponentRef ScnEntity::getComponent( BcU32 Idx )
{
	// HACK.
	if( Components_.size() > 0 )
	{
		BcAssert( Idx < Components_.size() );
		return Components_[ Idx ];
	}
	else
	{
		BcAssert( Idx < AttachComponents_.size() );
		return AttachComponents_[ Idx ];
	}
}

//////////////////////////////////////////////////////////////////////////
// setPosition
void ScnEntity::setPosition( const BcVec3d& Position )
{
	Transform_.translation( Position );
}

//////////////////////////////////////////////////////////////////////////
// setMatrix
void ScnEntity::setMatrix( const BcMat4d& Matrix )
{
	Transform_ = Matrix;
}

//////////////////////////////////////////////////////////////////////////
// getMatrix
const BcMat4d& ScnEntity::getMatrix() const
{
	return Transform_;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
BcAABB ScnEntity::getAABB()
{
	BcAABB AABB;
	
	for( ScnComponentListIterator It( Components_.begin() ); It != Components_.end(); ++It )
	{
		AABB.expandBy( (*It)->getAABB() );
	}
	
	return AABB;
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
	if( ChunkID == BcHash( "object" ) )
	{
		pJsonObject_ = reinterpret_cast< const BcChar* >( pData );
	}
}

//////////////////////////////////////////////////////////////////////////
// processAttachDetach
void ScnEntity::processAttachDetach()
{
	// Detach first.
	while( DetachComponents_.size() > 0 )
	{
		ScnComponentList DetachComponents( DetachComponents_ );
		DetachComponents_.clear();
		for( ScnComponentListIterator It( DetachComponents.begin() ); It != DetachComponents.end(); ++It )
		{
			internalDetach( (*It) );
		}
	}

	// Attach second.
	while( AttachComponents_.size() > 0 )
	{
		ScnComponentList AttachComponents( AttachComponents_ );
		AttachComponents_.clear();
		for( ScnComponentListIterator It( AttachComponents.begin() ); It != AttachComponents.end(); ++It )
		{
			internalAttach( (*It) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// internalAttach
void ScnEntity::internalAttach( ScnComponent* Component )
{
	BcAssertMsg( Component != NULL, "Trying to attach a null component!" );

	// If we're not attached to ourself, bail.
	if( Component->isAttached( this ) == BcFalse )
	{
		BcAssertMsg( Component->isAttached() == BcFalse, "Component is attached to another entity!" );

		// Call the on detach.
		Component->onAttach( ScnEntityWeakRef( this ) );

		// Put into component list.
		Components_.push_back( Component );

		// Tell the scene about it.
		if( isAttached() == BcTrue )
		{
			ScnCore::pImpl()->onAttachComponent( ScnEntityWeakRef( this ), Component );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// internalDetach
void ScnEntity::internalDetach( ScnComponent* Component )
{
	BcAssertMsg( Component != NULL, "Trying to detach a null component!" );

	// If component isn't attached, don't worry. Only a warning?
	if( Component->isAttached() == BcTrue )
	{
		BcAssertMsg( Component->isAttached( this ) == BcTrue, "Component isn't attached to this entity!" );
		// Call the on detach.
		Component->onDetach( ScnEntityWeakRef( this ) );

		// Remove from the list.
		for( ScnComponentListIterator It( Components_.begin() ); It != Components_.end(); ++It )
		{
			ScnComponentRef& ListComponent( *It );

			if( ListComponent == Component )
			{
				// Remove from component list.
				Components_.erase( It );
				break;
			}
		}

		// Tell the scene about it.
		if( isAttached() )
		{
			ScnCore::pImpl()->onDetachComponent( ScnEntityWeakRef( this ), Component );
		}
	}
}

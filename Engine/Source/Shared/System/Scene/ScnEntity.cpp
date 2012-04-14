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

//#include "System/Renderer/RsCore.h"

#ifdef PSY_SERVER
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnEntity::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
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
	pHeader_ = NULL;
	IsAttached_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEntity::initialise( ScnEntityRef Basis )
{
	// Keep a ref to our basis so it isn't unloaded.
	Basis_ = Basis;

	// Copy over internals.
	pHeader_ = Basis_->pHeader_;
	Transform_ = Basis_->Transform_;
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

	return BcTrue;
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
void ScnEntity::render( RsFrame* pFrame, RsRenderSort Sort )
{
	for( ScnComponentListIterator It( Components_.begin() ); It != Components_.end(); ++It )
	{
		ScnRenderableComponentWeakRef RenderableComponent( *It );
		if( RenderableComponent.isValid() )
		{
			RenderableComponent->render( pFrame, Sort );
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
	return IsAttached_;
}

//////////////////////////////////////////////////////////////////////////
// getNoofComponents
BcU32 ScnEntity::getNoofComponents() const
{
	return Components_.size();
}
	
//////////////////////////////////////////////////////////////////////////
// getComponent
ScnComponentRef ScnEntity::getComponent( BcU32 Idx )
{
	BcAssert( Idx < Components_.size() );
	return Components_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// setPosition
void ScnEntity::setPosition( const BcVec3d& Position )
{
	Transform_.setTranslation( Position );
}

//////////////////////////////////////////////////////////////////////////
// setRotation
void ScnEntity::setRotation( const BcQuat& Rotation )
{
	Transform_.setRotation( Rotation );
}

//////////////////////////////////////////////////////////////////////////
// setMatrix
void ScnEntity::setMatrix( const BcMat4d& Matrix )
{
	Transform_.setMatrix( Matrix );
}

//////////////////////////////////////////////////////////////////////////
// getTransform
const ScnTransform& ScnEntity::getTransform() const
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
	if( ChunkID == BcHash( "header" ) )
	{
		// Grab pointer to header.
		pHeader_ = reinterpret_cast< THeader* >( pData );
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
		if( IsAttached_ == BcTrue )
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
		if( IsAttached_ )
		{
			ScnCore::pImpl()->onDetachComponent( ScnEntityWeakRef( this ), Component );
		}
	}
}

/**************************************************************************
*
* File:		GaEntity.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaEntity.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaEntity );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaEntity::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void GaEntity::create()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void GaEntity::destroy()
{
	detachAllComponents();
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool GaEntity::isReady()
{
	BcBool IsReady = BcTrue;
	
	for( GaComponentListIterator It( ComponentList_.begin() ); It != ComponentList_.end(); ++It )
	{
		GaComponentRef& Component = (*It);
		
		if( Component->isReady() == BcFalse )
		{
			IsReady = BcFalse;
			break;
		}
	}
	
	return IsReady;
}

//////////////////////////////////////////////////////////////////////////
// attachComponent
void GaEntity::attachComponent( GaComponent* pComponent )
{
	// Call component pre-attach.
	pComponent->preAttach( this );
	
	// Put into list.
	ComponentList_.push_back( pComponent );
	
	// Call component post-attach. 
	pComponent->postAttach();
}

//////////////////////////////////////////////////////////////////////////
// detachComponent
void GaEntity::detachComponent( GaComponent* pComponent )
{
	// Call component pre-detach.
	pComponent->preDetach();

	// Find in list and remove.
	for( GaComponentListIterator It( ComponentList_.begin() ); It != ComponentList_.end(); ++It )
	{
		if( (*It) == pComponent )
		{
			ComponentList_.erase( It );
			break;
		}
	}
	
	// Call component post-detach. 
	pComponent->postDetach();
}

//////////////////////////////////////////////////////////////////////////
// detachAllComponents
void GaEntity::detachAllComponents()
{
	// Detach all components.
	while( ComponentList_.size() > 0 )
	{
		GaComponentRef& Component = *ComponentList_.begin();
		
		detachComponent( Component );		
	}
}

//////////////////////////////////////////////////////////////////////////
// setTransform
void GaEntity::setTransform( const GaTransform& Transform )
{
	Transform_ = Transform;
}

//////////////////////////////////////////////////////////////////////////
// getTransform
const GaTransform& GaEntity::getTransform() const
{
	return Transform_;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaEntity::update( BcReal Tick )
{
	// Update components.
	for( GaComponentListIterator It( ComponentList_.begin() ); It != ComponentList_.end(); ++It )
	{
		GaComponentRef& Component = (*It);
		
		Component->update( Tick );
	}
}

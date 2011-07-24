/**************************************************************************
*
* File:		GaComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/07/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "GaComponent.h"

#include "GaEntity.h"

//////////////////////////////////////////////////////////////////////////
// reAttach
void GaComponent::reAttach()
{
	GaEntity* pOwner = pOwner_;
	
	pOwner->detachComponent( this );
	pOwner->attachComponent( this );
}

//////////////////////////////////////////////////////////////////////////
// preAttach
//virtual
void GaComponent::preAttach( GaEntity* pOwner )
{
	BcAssertMsg( pOwner_ == NULL, "Component must not be attached to an entity on attach." );
	pOwner_ = pOwner;
}

//////////////////////////////////////////////////////////////////////////
// postAttach
//virtual
void GaComponent::postAttach()
{
	
}

//////////////////////////////////////////////////////////////////////////
// preDetach
//virtual
void GaComponent::preDetach()
{
	BcAssertMsg( pOwner_ != NULL, "Component must be attached to an entity to detach." );

}

//////////////////////////////////////////////////////////////////////////
// postDetach
//virtual
void GaComponent::postDetach()
{
	BcAssertMsg( pOwner_ != NULL, "Component must be attached to an entity to detach." );
	pOwner_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaComponent::update( BcReal Tick )
{
	BcAssertMsg( pOwner_ != NULL, "Can't tick component without owner." );
	
}

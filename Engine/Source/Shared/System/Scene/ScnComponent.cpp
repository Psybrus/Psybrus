/**************************************************************************
*
* File:		ScnComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	26/11/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnComponent.h"
#include "RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnComponent" )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnComponent::update( BcReal Tick )
{
	
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnComponent::onAttach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( ParentEntity_.isValid() == BcFalse, "Attempting to attach component when it's already attached!" );

	ParentEntity_ = Parent;
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnComponent::onDetach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( ParentEntity_.isValid() == BcTrue, "Attempting to detach component that is already detached!" );
	BcAssertMsg( ParentEntity_ == Parent, "Attempting to detach component from an entity it isn't attached to!" );

	ParentEntity_ = NULL;
}

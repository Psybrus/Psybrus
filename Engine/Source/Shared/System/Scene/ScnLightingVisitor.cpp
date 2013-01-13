/**************************************************************************
*
* File:		ScnLightingVisitor.cpp
* Author: 	Neil Richardson 
* Ver/Date:	13/01/13
* Description:
*		Lighting visitor for the scenegraph.
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnLightingVisitor.h"

#include "System/Scene/ScnRenderableComponent.h"
#include "System/Scene/ScnLightComponent.h"
#include "System/Scene/ScnMaterial.h"

#include "System/Scene/ScnCore.h"

//////////////////////////////////////////////////////////////////////////
// ScnLightingVisitorLightSort
class ScnLightingVisitorLightSort
{
public:
	ScnLightingVisitorLightSort( const BcVec3d& Position ):
		Position_( Position )
	{
	}

	bool operator()( class ScnLightComponent*& A, class ScnLightComponent*& B )
	{
		BcF32 DistanceA = ( Position_ - A->getParentEntity()->getWorldPosition() ).magnitude();
		BcF32 DistanceB = ( Position_ - B->getParentEntity()->getWorldPosition() ).magnitude();
		BcF32 AttnA = A->findAttenuationByDistance( DistanceA );
		BcF32 AttnB = B->findAttenuationByDistance( DistanceB );
		return AttnA > AttnB;
	}

private:
	BcVec3d Position_;
};

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnLightingVisitor::ScnLightingVisitor( class ScnRenderableComponent* RenderableComponent )
{
	LightComponents_.reserve( MAX_LIGHTS );
	ScnCore::pImpl()->visitBounds( this, RenderableComponent->getAABB() );

	// Sort by light strength
	std::sort( LightComponents_.begin(), LightComponents_.end(), ScnLightingVisitorLightSort( RenderableComponent->getAABB().centre() ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnLightingVisitor::~ScnLightingVisitor()
{

}

//////////////////////////////////////////////////////////////////////////
// visit
//virtual
void ScnLightingVisitor::visit( class ScnLightComponent* Component )
{
	if( LightComponents_.size() < MAX_LIGHTS )
	{
		LightComponents_.push_back( Component );
	}
}

//////////////////////////////////////////////////////////////////////////
// setMaterialParameters
void ScnLightingVisitor::setMaterialParameters( class ScnMaterialComponent* MaterialComponent ) const
{
	BcU32 Count = BcMin( (BcU32)4, (BcU32)LightComponents_.size() );
	for( BcU32 Idx = 0; Idx < Count; ++Idx )
	{
		LightComponents_[ Idx ]->setMaterialParameters( Idx, MaterialComponent );
	}	

	for( BcU32 Idx = Count; Idx < 4; ++Idx )
	{
		MaterialComponent->setLightParameters( Idx,
	                                           BcVec3d( 0.0f, 0.0f, 0.0f ),
	                                           BcVec3d( 0.0f, 0.0f, 0.0f ),
	                                           RsColour::BLACK,
	                                           RsColour::BLACK,
	                                           0.0f,
	                                           0.0f,
	                                           0.0f );
	}
}

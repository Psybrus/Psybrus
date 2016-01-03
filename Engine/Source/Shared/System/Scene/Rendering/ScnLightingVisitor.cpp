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

#include "System/Scene/Rendering/ScnLightingVisitor.h"

#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnLightComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"

#include "System/Scene/ScnCore.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// ScnLightingVisitorLightSort
class ScnLightingVisitorLightSort
{
public:
	ScnLightingVisitorLightSort( const MaVec3d& Position ):
		Position_( Position )
	{
	}

	bool operator()( const class ScnLightComponent* A, const class ScnLightComponent* B ) const
	{
		BcF32 DistanceA = ( Position_ - A->getParentEntity()->getWorldPosition() ).magnitude();
		BcF32 DistanceB = ( Position_ - B->getParentEntity()->getWorldPosition() ).magnitude();
		BcF32 AttnA = A->findAttenuationByDistance( DistanceA );
		BcF32 AttnB = B->findAttenuationByDistance( DistanceB );
		return AttnA > AttnB;
	}

private:
	MaVec3d Position_;
};

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnLightingVisitor::ScnLightingVisitor( class ScnRenderableComponent* RenderableComponent )
{
	PSY_PROFILE_FUNCTION;
	ScnCore::pImpl()->visitBounds( this, RenderableComponent->getAABB() );

	// Sort by light strength
	std::sort( LightComponents_.begin(), LightComponents_.end(), ScnLightingVisitorLightSort( RenderableComponent->getAABB().centre() ) );

	// Build uniform block.
	BcU32 Count = BcMin( ScnShaderLightUniformBlockData::MAX_LIGHTS, LightComponents_.size() );
	for( BcU32 Idx = 0; Idx < Count; ++Idx )
	{
		ScnLightComponent* LightComponent = LightComponents_[ Idx ];
		LightComponent->setLightUniformBlockData( Idx, LightUniformData_ );
	}

	// Clear unused lights.
	for( BcU32 Idx = Count; Idx < 4; ++Idx )
	{
		LightUniformData_.LightPosition_[ Idx ] = MaVec3d( 0.0f, 0.0f, 0.0f );
		LightUniformData_.LightDirection_[ Idx ] = MaVec3d( 0.0f, 0.0f, 0.0f );
		LightUniformData_.LightAmbientColour_[ Idx ] = RsColour::BLACK;
		LightUniformData_.LightDiffuseColour_[ Idx ] = RsColour::BLACK;
		LightUniformData_.LightAttn_[ Idx ] = MaVec4d( 0.0, 0.0, 0.0, 0.0f );
	}
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
	PSY_PROFILE_FUNCTION;
	if( NoofLights_ < LightComponents_.size() )
	{
		LightComponents_[ NoofLights_++ ] = Component;
	}
}

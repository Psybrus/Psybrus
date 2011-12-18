/**************************************************************************
*
* File:		RsViewport.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsViewport.h"

//////////////////////////////////////////////////////////////////////////
// Constructor
RsViewport::RsViewport()
{
	ClearDepth_ = BcFalse;
	ZNear_ = 1.0f; // hack
	ZFar_ = 1024.0f;
}

//////////////////////////////////////////////////////////////////////////
// Destructor
RsViewport::~RsViewport()
{

}

//////////////////////////////////////////////////////////////////////////
// unProject
void RsViewport::unProject( const BcVec2d& ScreenCoord, BcVec3d& WorldNearPos, BcVec3d& WorldFarPos ) const
{
	BcVec2d Screen = ScreenCoord - BcVec2d( static_cast<BcReal>(x()), static_cast<BcReal>(y()) );
	const BcVec2d RealScreen( ( Screen.x() / width() ) * 2.0f - 1.0f, ( Screen.y() / height() ) * 2.0f - 1.0f );

	BcMat4d InvViewMat = view();
	BcMat4d InvProjMat = projection();

	InvViewMat.inverse();
	InvProjMat.inverse();

	WorldNearPos.set( RealScreen.x(), -RealScreen.y(), 0.0f );
	WorldFarPos.set( RealScreen.x(), -RealScreen.y(), 1.0f );

	WorldNearPos = WorldNearPos * InvProjMat;
	WorldFarPos = WorldFarPos * InvProjMat;

	if( Projection_[3][3] == 0.0f )
	{
		WorldNearPos *= ZNear_;
		WorldFarPos *= ZFar_;
	}

	WorldNearPos = WorldNearPos * InvViewMat;
	WorldFarPos = WorldFarPos * InvViewMat;
}

//////////////////////////////////////////////////////////////////////////
// project
BcVec2d RsViewport::project( const BcVec3d& WorldPos )
{
	BcVec4d ScreenSpace = BcVec4d( WorldPos, 1.0f ) * viewProjection();
	BcVec2d ScreenPosition = BcVec2d( ScreenSpace.x() / ScreenSpace.w(), -ScreenSpace.y() / ScreenSpace.w() );

	BcReal HalfW = BcReal( width() ) * 0.5f;
	BcReal HalfH = BcReal( height() ) * 0.5f;
	return BcVec2d( ( ScreenPosition.x() * HalfW ), ( ScreenPosition.y() * HalfH ) );
}

//////////////////////////////////////////////////////////////////////////
// depth
BcU32 RsViewport::depth( const BcVec3d& WorldPos ) const
{
	BcVec4d ScreenSpace = BcVec4d( WorldPos, 1.0f ) * viewProjection();
	BcReal Depth = 1.0f - BcClamp( ScreenSpace.z() / ScreenSpace.w(), 0.0f, 1.0f );

	return BcU32( Depth * BcReal( 0xffffff ) );
}

//////////////////////////////////////////////////////////////////////////
// intersect
BcBool RsViewport::intersect( const BcAABB& AABB ) const
{
	BcVec3d Centre = AABB.centre();
	BcReal Radius = ( AABB.max() - AABB.min() ).magnitude() * 0.5f;

	BcReal Distance;
	for( BcU32 i = 0; i < 6; ++i )
	{
		Distance = FrustumPlanes_[ i ].distance( Centre );
		if( Distance > Radius )
		{
			return BcFalse;
		}
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// buildFrustum
void RsViewport::buildFrustum()
{
	FrustumPlanes_[ 0 ] = BcPlane( ( ViewProj_[0][3] + ViewProj_[0][0] ),
	                               ( ViewProj_[1][3] + ViewProj_[1][0] ),
	                               ( ViewProj_[2][3] + ViewProj_[2][0] ),
	                               ( ViewProj_[3][3] + ViewProj_[3][0]) );

	FrustumPlanes_[ 1 ] = BcPlane( ( ViewProj_[0][3] - ViewProj_[0][0] ),
	                               ( ViewProj_[1][3] - ViewProj_[1][0] ),
	                               ( ViewProj_[2][3] - ViewProj_[2][0] ),
	                               ( ViewProj_[3][3] - ViewProj_[3][0] ) );

	FrustumPlanes_[ 2 ] = BcPlane( ( ViewProj_[0][3] + ViewProj_[0][1] ),
	                               ( ViewProj_[1][3] + ViewProj_[1][1] ),
	                               ( ViewProj_[2][3] + ViewProj_[2][1] ),
	                               ( ViewProj_[3][3] + ViewProj_[3][1] ) );

	FrustumPlanes_[ 3 ] = BcPlane( ( ViewProj_[0][3] - ViewProj_[0][1] ),
	                               ( ViewProj_[1][3] - ViewProj_[1][1] ),
	                               ( ViewProj_[2][3] - ViewProj_[2][1] ),
	                               ( ViewProj_[3][3] - ViewProj_[3][1] ) );

	FrustumPlanes_[ 4 ] = BcPlane( ( ViewProj_[0][3] - ViewProj_[0][2] ),
	                               ( ViewProj_[1][3] - ViewProj_[1][2] ),
	                               ( ViewProj_[2][3] - ViewProj_[2][2] ),
	                               ( ViewProj_[3][3] - ViewProj_[3][2] ) );
	
	FrustumPlanes_[ 5 ] = BcPlane( ( ViewProj_[0][3] ),
	                               ( ViewProj_[1][3] ),
	                               ( ViewProj_[2][3] ),
	                               ( ViewProj_[3][3] ) );

	// Normalise planes.
	for ( BcU32 i = 0; i < 6; ++i )
	{
		BcVec3d Normal = FrustumPlanes_[ i ].normal();
		BcReal Scale = 1.0f / -Normal.magnitude();
		FrustumPlanes_[ i ] = BcPlane( FrustumPlanes_[ i ].normal().x() * Scale,
		                               FrustumPlanes_[ i ].normal().y() * Scale,
		                               FrustumPlanes_[ i ].normal().z() * Scale,
		                               FrustumPlanes_[ i ].d() * Scale );
	}
}

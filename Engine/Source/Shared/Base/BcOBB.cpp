/**************************************************************************
*
* File:		BcOBB.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Oriented Bounding Box
*		
*
*
* 
**************************************************************************/

#include "Base/BcOBB.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
BcOBB::BcOBB()
{
	empty();
}

BcOBB::BcOBB( const BcOBB& Other )
{
	for( BcU32 i = 0; i < 8; ++i )
	{
		Corners_[ i ] = Other.Corners_[ i ];
	}
}

BcOBB::BcOBB( const BcAABB& AABB, const BcMat4d& Transform )
{
	for( BcU32 i = 0; i < 8; ++i )
	{
		Corners_[ i ] = AABB.corner( i ) * Transform;
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcOBB::~BcOBB()
{

}

//////////////////////////////////////////////////////////////////////////
// facePlane
BcPlane BcOBB::facePlane( BcU32 i ) const
{
	BcPlane Plane;
	BcVec3d Point;
	BcVec3d Normal;

	switch( i )
	{
	case FRONT:
		Plane.fromPoints( corner( LTF ), corner( LBF ), corner( RBF ) );
		break;
	case BACK:
		Plane.fromPoints( corner( RTB ), corner( RBB ), corner( LBB ) );
		break;
	case TOP:
		Plane.fromPoints( corner( LTF ), corner( RTF ), corner( RTB ) );
		break;
	case BOTTOM:
		Plane.fromPoints( corner( LBB ), corner( RBB ), corner( RBF ) );
		break;
	case LEFT:
		Plane.fromPoints( corner( LBB ), corner( LBF ), corner( LTF ) );
		break;
	case RIGHT:
		Plane.fromPoints( corner( RTB ), corner( RTF ), corner( RBF ) );
		break;
	default:
		BcBreakpoint;
	}

	return Plane;
}

//////////////////////////////////////////////////////////////////////////
// faceCentre
BcVec3d BcOBB::faceCentre( BcU32 i ) const
{
	BcVec3d FaceCentre = centre();
	switch( i )
	{
	case FRONT:
	case BACK:
		FaceCentre += facePlane( i ).normal() * ( depth() * 0.5f );
		break;
	case TOP:
	case BOTTOM:
		FaceCentre += facePlane( i ).normal() * ( height() * 0.5f );
		break;
	case LEFT:
	case RIGHT:
		FaceCentre += facePlane( i ).normal() * ( width() * 0.5f );
		break;
	default:
		BcBreakpoint;
	}

	return FaceCentre;
}

//////////////////////////////////////////////////////////////////////////
// centre
BcVec3d BcOBB::centre() const
{
	BcVec3d Centre( 0.0f, 0.0f, 0.0f );

	for( BcU32 i = 0; i < 8; ++i )
	{
		Centre += corner( i );
	}

	Centre = Centre * 0.125f;

	return Centre;
}

//////////////////////////////////////////////////////////////////////////
// rayIntersect
BcBool BcOBB::lineIntersect( const BcVec3d& Start, const BcVec3d& End, BcVec3d* pIntersectionPoint, BcVec3d* pIntersectionNormal ) const
{
	// Planes. Screw it.
	// Totally inoptimal.
	BcPlane Planes[6];
	BcVec3d Intersects[6];
	BcF32 Distance;
	for( BcU32 i = 0; i < 6; ++i )
	{
		Planes[i] = facePlane( i );
		if( !Planes[i].lineIntersection( Start, End, Distance, Intersects[i] ) )
		{
			Intersects[i] = BcVec3d( 1e24f, 1e24f, 1e24f );
		}
	}

	// Reject classified and find nearest.
	BcF32 Nearest = 1e24f;
	BcU32 iNearest = BcErrorCode;

	for( BcU32 i = 0; i < 6; ++i )
	{
		// For every point...
		// ...check against planes.
		BcBool Valid = BcTrue;
		for( BcU32 j = 0; j < 6; ++j )
		{
			if( Planes[j].classify( Intersects[i] ) == BcPlane::bcPC_FRONT )
			{
				Valid = BcFalse;
				break;
			}
		}

		// If its valid, check distance.
		if( Valid )
		{
			BcF32 Distance = ( Start - Intersects[i] ).magnitudeSquared();

			if( Distance < Nearest )
			{
				Nearest = Distance;
				iNearest = i;
			}
		}
	}

	//
	if( iNearest != BcErrorCode )
	{
		if( pIntersectionPoint != NULL )
		{
			*pIntersectionPoint = Intersects[ iNearest ];
		}

		if( pIntersectionNormal != NULL )
		{
			*pIntersectionNormal = Planes[ iNearest ].normal();
		}

		return BcTrue;
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// empty
void BcOBB::empty()
{
	for( BcU32 i = 0; i < 8; ++i )
	{
		Corners_[ i ].set( 0.0f, 0.0f, 0.0f );
	}
}

//////////////////////////////////////////////////////////////////////////
// transform
BcOBB BcOBB::transform( const BcMat4d& Transform ) const
{
	BcOBB NewOBB;

	for( BcU32 i = 0; i < 8; ++i )
	{
		NewOBB.Corners_[ i ] = Corners_[ i ] * Transform;
	}

	return NewOBB;
}

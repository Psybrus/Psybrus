/**************************************************************************
*
* File:		MaAABB.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Axis Aligned Bounding Box
*		
*
*
* 
**************************************************************************/

#include "Math/MaAABB.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
MaAABB::MaAABB()
{
	empty();
}

MaAABB::MaAABB( const MaAABB& Other ):
	Min_( Other.Min_ ),
	Max_( Other.Max_ )
{
	
}

MaAABB::MaAABB( const MaVec3d& Min, const MaVec3d& Max ):
	Min_( Min ),
	Max_( Max )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
MaAABB::~MaAABB()
{

}

//////////////////////////////////////////////////////////////////////////
//
MaPlane MaAABB::facePlane( BcU32 i ) const
{
	MaPlane Plane;
	MaVec3d Point;
	MaVec3d Normal;

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
MaVec3d MaAABB::faceCentre( BcU32 i ) const
{
	MaVec3d FaceCentre = centre();
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
// rayIntersect
BcBool MaAABB::lineIntersect( const MaVec3d& Start, const MaVec3d& End, MaVec3d* pIntersectionPoint, MaVec3d* pIntersectionNormal ) const
{
	// Planes. Screw it.
	// Totally inoptimal.
	MaPlane Planes[6];
	MaVec3d Intersects[6];
	BcF32 Distance;
	for( BcU32 i = 0; i < 6; ++i )
	{
		Planes[i] = facePlane( i );
		if( !Planes[i].lineIntersection( Start, End, Distance, Intersects[i] ) )
		{
			Intersects[i] = MaVec3d( 1e24f, 1e24f, 1e24f );
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
			if( Planes[j].classify( Intersects[i] ) == MaPlane::bcPC_FRONT )
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
//
BcBool MaAABB::boxIntersect( const MaAABB& AABB, MaAABB* pIntersectionBox ) const
{
	// Check for no overlap.
	if( ( Min_.x() > AABB.Max_.x() ) ||
		( Max_.x() < AABB.Min_.x() ) ||
		( Min_.y() > AABB.Max_.y() ) ||
		( Max_.y() < AABB.Min_.y() ) ||
		( Min_.z() > AABB.Max_.z() ) ||
		( Max_.z() < AABB.Min_.z() ) )
	{
		return BcFalse;
	}

	// Overlap, compute AABB of intersection.
	if( pIntersectionBox != NULL )
	{
		pIntersectionBox->Min_.x( BcMax( Min_.x(), AABB.Min_.x() ) );
		pIntersectionBox->Max_.x( BcMin( Max_.x(), AABB.Max_.x() ) );
		pIntersectionBox->Min_.y( BcMax( Min_.y(), AABB.Min_.y() ) );
		pIntersectionBox->Max_.y( BcMin( Max_.y(), AABB.Max_.y() ) );
		pIntersectionBox->Min_.z( BcMax( Min_.z(), AABB.Min_.z() ) );
		pIntersectionBox->Max_.z( BcMin( Max_.z(), AABB.Max_.z() ) );
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// classify
MaAABB::eClassify MaAABB::classify( const MaVec3d& Point ) const
{
	if( ( Point.x() >= Min_.x() && Point.x() <= Max_.x() ) &&
		( Point.y() >= Min_.y() && Point.y() <= Max_.y() ) &&
		( Point.z() >= Min_.z() && Point.z() <= Max_.z() ) )
	{
		return bcBC_INSIDE;
	}

	return bcBC_OUTSIDE;
}

//////////////////////////////////////////////////////////////////////////
// classify
MaAABB::eClassify MaAABB::classify( const MaAABB& AABB ) const
{
	BcU32 PointsInside = 0;
	for( BcU32 i = 0; i < 8; ++i )
	{
		MaVec3d Point = AABB.corner( i );
		if( ( Point.x() >= Min_.x() && Point.x() <= Max_.x() ) &&
			( Point.y() >= Min_.y() && Point.y() <= Max_.y() ) &&
			( Point.z() >= Min_.z() && Point.z() <= Max_.z() ) )
		{
			PointsInside++;
		}
	}

	if( PointsInside == 8 )
	{
		return bcBC_INSIDE;
	}

	if( PointsInside > 0 )
	{
		return bcBC_SPANNING;
	}

	return bcBC_OUTSIDE;
}

//////////////////////////////////////////////////////////////////////////
// transform
MaAABB MaAABB::transform( const MaMat4d& Transform ) const
{
	MaAABB NewAABB;

	// Add transformed corners.
	NewAABB.expandBy( corner( 0 ) * Transform );
	NewAABB.expandBy( corner( 1 ) * Transform );
	NewAABB.expandBy( corner( 2 ) * Transform );
	NewAABB.expandBy( corner( 3 ) * Transform );
	NewAABB.expandBy( corner( 4 ) * Transform );
	NewAABB.expandBy( corner( 5 ) * Transform );
	NewAABB.expandBy( corner( 6 ) * Transform );
	NewAABB.expandBy( corner( 7 ) * Transform );

	return NewAABB;
}

void MaAABB::min( const MaVec3d& Min )
{
	Min_ = Min;
}

void MaAABB::max( const MaVec3d& Max )
{
	Max_ = Max;
}

const MaVec3d& MaAABB::min() const
{
	return Min_;
}

const MaVec3d& MaAABB::max() const
{
	return Max_;
}

MaVec3d MaAABB::corner( BcU32 i ) const
{
	return MaVec3d( ( i & 1 ) ? Min_.x() : Max_.x(),
	                ( i & 2 ) ? Min_.y() : Max_.y(),
	                ( i & 4 ) ? Min_.z() : Max_.z() );
}

MaVec3d MaAABB::centre() const
{
	return ( ( Min_ + Max_ ) * 0.5f );
}

MaVec3d MaAABB::dimensions() const
{
	return ( Max_ - Min_ );
}

BcF32 MaAABB::diameter() const
{
	return ( ( Max_ - Min_ ).magnitude() );
}

void MaAABB::empty()
{
	Min_.set( 1e24f, 1e24f, 1e24f );
	Max_.set( -1e24f, -1e24f, -1e24f );
}

BcBool MaAABB::isEmpty() const
{
	return ( ( Min_.x() > Max_.x() ) ||
	         ( Min_.y() > Max_.y() ) ||
	         ( Min_.z() > Max_.z() ) );
}

void MaAABB::expandBy( const MaVec3d& Point )
{
	Min_.x( BcMin( Min_.x(), Point.x() ) );
	Min_.y( BcMin( Min_.y(), Point.y() ) );
	Min_.z( BcMin( Min_.z(), Point.z() ) );

	Max_.x( BcMax( Max_.x(), Point.x() ) );
	Max_.y( BcMax( Max_.y(), Point.y() ) );
	Max_.z( BcMax( Max_.z(), Point.z() ) );
}

void MaAABB::expandBy( const MaAABB& AABB )
{
	Min_.x( BcMin( Min_.x(), AABB.Min_.x() ) );
	Min_.y( BcMin( Min_.y(), AABB.Min_.y() ) );
	Min_.z( BcMin( Min_.z(), AABB.Min_.z() ) );

	Max_.x( BcMax( Max_.x(), AABB.Max_.x() ) );
	Max_.y( BcMax( Max_.y(), AABB.Max_.y() ) );
	Max_.z( BcMax( Max_.z(), AABB.Max_.z() ) );
}

BcF32 MaAABB::width() const
{
	return Max_.x() - Min_.x();
}

BcF32 MaAABB::height() const
{
	return Max_.y() - Min_.y();
}

BcF32 MaAABB::depth() const
{
	return Max_.z() - Min_.z();
}

BcF32 MaAABB::volume() const
{
	return ( width() * height() * depth() );
}

/**************************************************************************
*
* File:		BcPlane.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*
*
*
* 
**************************************************************************/

#include "Base/BcVectors.h"
#include "Base/BcPlane.h"

#include "Base/BcMath.h"

const BcReal gPlaneEpsilon = BcVecEpsilon;

//////////////////////////////////////////////////////////////////////////
// pointClassify
void BcPlane::transform( const BcMat4d& Transform )
{
	BcVec3d Translation( Transform.row3().x(), Transform.row3().y(), Transform.row3().z() );
	Normal_ = ( Normal_ * Transform ) - Translation;
	D_ = D_ - ( Normal_.dot( Translation ) );
}

//////////////////////////////////////////////////////////////////////////
// pointClassify
BcPlane::eClassify BcPlane::classify( const BcVec3d& Point ) const
{
	BcReal Dist = distance( Point );
			
	if( Dist > gPlaneEpsilon )
	{
		return bcPC_FRONT;
	}
	else if( Dist < -gPlaneEpsilon )
	{
		return bcPC_BACK;
	}

	return bcPC_COINCIDING;
}

//////////////////////////////////////////////////////////////////////////
// lineIntersection
BcBool BcPlane::lineIntersection( const BcVec3d& Point, const BcVec3d& Dir, BcReal& Distance ) const
{
	const BcReal Dist = ( Normal_.dot( Point ) ) + D_;
	const BcReal Ndiv = Normal_.dot( Dir );

	if( BcAbs( Ndiv ) > 0.0f )
	{
		Distance = Dist / Ndiv;
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// lineIntersection
BcBool BcPlane::lineIntersection( const BcVec3d& A, const BcVec3d& B, BcReal& Distance, BcVec3d& Intersection ) const
{
	BcBool RetVal = BcFalse;
	if( lineIntersection( A, A - B, Distance ) && Distance >= 0.0f && Distance <= 1.0f )
	{
		Intersection = A + ( ( B - A ) * Distance );
		RetVal = BcTrue;
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// intersect
BcBool BcPlane::intersect( const BcPlane& A, const BcPlane& B, const BcPlane& C, BcVec3d& Point )
{
	const BcReal Denom = A.Normal_.dot( ( B.Normal_.cross( C.Normal_ ) ) );

	if ( BcAbs( Denom ) < ( gPlaneEpsilon ) )
	{
		return BcFalse;
	}

	Point = ( ( ( B.Normal_.cross( C.Normal_ ) ) * -A.D_ ) -
	          ( ( C.Normal_.cross( A.Normal_ ) ) *  B.D_ ) -
	          ( ( A.Normal_.cross( B.Normal_ ) ) *  C.D_ ) ) / Denom;

	return BcTrue;
}

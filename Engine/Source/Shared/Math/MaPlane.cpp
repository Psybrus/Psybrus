/**************************************************************************
*
* File:		MaPlane.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*
*
*
* 
**************************************************************************/

#include "Math/MaPlane.h"

#include "Base/BcMath.h"

const BcF32 gPlaneEpsilon = BcVecEpsilon;

REFLECTION_DEFINE_BASIC( MaPlane );

void MaPlane::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Normal_",		&MaPlane::Normal_ ),
		new ReField( "D_",			&MaPlane::D_ ),
	};
		
	ReRegisterClass< MaPlane >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// pointClassify
void MaPlane::transform( const MaMat4d& Transform )
{
	MaVec3d Translation( Transform.row3().x(), Transform.row3().y(), Transform.row3().z() );
	Normal_ = ( Normal_ * Transform ) - Translation;
	D_ = D_ - ( Normal_.dot( Translation ) );
}

//////////////////////////////////////////////////////////////////////////
// pointClassify
MaPlane::eClassify MaPlane::classify( const MaVec3d& Point, BcF32 Radius ) const
{
	BcF32 Dist = distance( Point );
			
	if( Dist > Radius )
	{
		return bcPC_FRONT;
	}
	else if( Dist < -Radius )
	{
		return bcPC_BACK;
	}

	return bcPC_COINCIDING;
}

//////////////////////////////////////////////////////////////////////////
// lineIntersection
BcBool MaPlane::lineIntersection( const MaVec3d& Point, const MaVec3d& Dir, BcF32& Distance ) const
{
	const BcF32 Dist = ( Normal_.dot( Point ) ) + D_;
	const BcF32 Ndiv = Normal_.dot( -Dir );

	if( BcAbs( Ndiv ) > 0.0f )
	{
		Distance = Dist / Ndiv;
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// lineIntersection
BcBool MaPlane::lineIntersection( const MaVec3d& A, const MaVec3d& B, BcF32& Distance, MaVec3d& Intersection ) const
{
	BcBool RetVal = BcFalse;
	MaVec3d Dir = B - A;
	if( lineIntersection( A, Dir, Distance ) && Distance >= 0.0f && Distance <= 1.0f )
	{
		Intersection = A + ( Dir * Distance );
		RetVal = BcTrue;
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// intersect
BcBool MaPlane::intersect( const MaPlane& A, const MaPlane& B, const MaPlane& C, MaVec3d& Point )
{
	const BcF32 Denom = A.Normal_.dot( ( B.Normal_.cross( C.Normal_ ) ) );

	if ( BcAbs( Denom ) < ( gPlaneEpsilon ) )
	{
		return BcFalse;
	}

	Point = ( ( ( B.Normal_.cross( C.Normal_ ) ) * -A.D_ ) -
	          ( ( C.Normal_.cross( A.Normal_ ) ) *  B.D_ ) -
	          ( ( A.Normal_.cross( B.Normal_ ) ) *  C.D_ ) ) / Denom;

	return BcTrue;
}

void MaPlane::fromPoints( const MaVec3d& V1, const MaVec3d& V2, const MaVec3d& V3 )
{
	Normal_ = ( V1 - V2 ).cross( ( V3 - V2 ) );
	Normal_.normalise();
	D_ = -( V1.dot( Normal_ ) );
}

void MaPlane::fromPointNormal( const MaVec3d& Point, const MaVec3d& Normal )
{
	Normal_ = Normal;
	Normal_.normalise();
	D_ = -( Point.dot( Normal_ ) );
}

BcF32 MaPlane::distance( const MaVec3d& P ) const
{
	return ( Normal_.dot(P) ) + D_;
}

void MaPlane::normalise()
{
	D_ = D_ / Normal_.magnitude();
	Normal_.normalise();
};

const MaVec3d& MaPlane::normal() const
{
	return Normal_;
}

BcF32 MaPlane::d() const
{
	return D_;
};

BcBool MaPlane::operator == (const MaPlane& Other ) const
{
	return ( Other.normal() == normal() && Other.d() == d() );
}

MaPlane MaPlane::operator -() const
{
	return MaPlane( -Normal_, D_ );
}


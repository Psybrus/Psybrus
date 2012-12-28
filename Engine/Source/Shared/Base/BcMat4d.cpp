/**************************************************************************
*
* File:		BcMat4d.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "Base/BcMat3d.h"
#include "Base/BcMat4d.h"


BcMat4d BcMat4d::operator + ( const BcMat4d& Rhs )
{
	return BcMat4d( Row0_ + Rhs.Row0_,
	                Row1_ + Rhs.Row1_,
	                Row2_ + Rhs.Row2_,
	                Row3_ + Rhs.Row3_ );
}

BcMat4d BcMat4d::operator - ( const BcMat4d& Rhs )
{
	return BcMat4d( Row0_ - Rhs.Row0_,
	                Row1_ - Rhs.Row1_,
	                Row2_ - Rhs.Row2_,
	                Row3_ - Rhs.Row3_ );
}

BcMat4d BcMat4d::operator * ( BcF32 Rhs )
{
	return BcMat4d( Row0_ * Rhs,
	                Row1_ * Rhs,
	                Row2_ * Rhs,
	                Row3_ * Rhs );
}

BcMat4d BcMat4d::operator / ( BcF32 Rhs )
{
	return BcMat4d( Row0_ / Rhs,
	                Row1_ / Rhs,
	                Row2_ / Rhs,
	                Row3_ / Rhs );
}

BcMat4d BcMat4d::operator * ( const BcMat4d& Rhs ) const
{
	const BcMat4d& Lhs = (*this);

	return BcMat4d( Lhs[0][0] * Rhs[0][0] + Lhs[0][1] * Rhs[1][0] + Lhs[0][2] * Rhs[2][0] + Lhs[0][3] * Rhs[3][0],
	                Lhs[0][0] * Rhs[0][1] + Lhs[0][1] * Rhs[1][1] + Lhs[0][2] * Rhs[2][1] + Lhs[0][3] * Rhs[3][1],
	                Lhs[0][0] * Rhs[0][2] + Lhs[0][1] * Rhs[1][2] + Lhs[0][2] * Rhs[2][2] + Lhs[0][3] * Rhs[3][2],
	                Lhs[0][0] * Rhs[0][3] + Lhs[0][1] * Rhs[1][3] + Lhs[0][2] * Rhs[2][3] + Lhs[0][3] * Rhs[3][3],
	                Lhs[1][0] * Rhs[0][0] + Lhs[1][1] * Rhs[1][0] + Lhs[1][2] * Rhs[2][0] + Lhs[1][3] * Rhs[3][0],
	                Lhs[1][0] * Rhs[0][1] + Lhs[1][1] * Rhs[1][1] + Lhs[1][2] * Rhs[2][1] + Lhs[1][3] * Rhs[3][1],
	                Lhs[1][0] * Rhs[0][2] + Lhs[1][1] * Rhs[1][2] + Lhs[1][2] * Rhs[2][2] + Lhs[1][3] * Rhs[3][2],
	                Lhs[1][0] * Rhs[0][3] + Lhs[1][1] * Rhs[1][3] + Lhs[1][2] * Rhs[2][3] + Lhs[1][3] * Rhs[3][3],
	                Lhs[2][0] * Rhs[0][0] + Lhs[2][1] * Rhs[1][0] + Lhs[2][2] * Rhs[2][0] + Lhs[2][3] * Rhs[3][0],
	                Lhs[2][0] * Rhs[0][1] + Lhs[2][1] * Rhs[1][1] + Lhs[2][2] * Rhs[2][1] + Lhs[2][3] * Rhs[3][1],
	                Lhs[2][0] * Rhs[0][2] + Lhs[2][1] * Rhs[1][2] + Lhs[2][2] * Rhs[2][2] + Lhs[2][3] * Rhs[3][2],
	                Lhs[2][0] * Rhs[0][3] + Lhs[2][1] * Rhs[1][3] + Lhs[2][2] * Rhs[2][3] + Lhs[2][3] * Rhs[3][3],
	                Lhs[3][0] * Rhs[0][0] + Lhs[3][1] * Rhs[1][0] + Lhs[3][2] * Rhs[2][0] + Lhs[3][3] * Rhs[3][0],
	                Lhs[3][0] * Rhs[0][1] + Lhs[3][1] * Rhs[1][1] + Lhs[3][2] * Rhs[2][1] + Lhs[3][3] * Rhs[3][1],
	                Lhs[3][0] * Rhs[0][2] + Lhs[3][1] * Rhs[1][2] + Lhs[3][2] * Rhs[2][2] + Lhs[3][3] * Rhs[3][2],
	                Lhs[3][0] * Rhs[0][3] + Lhs[3][1] * Rhs[1][3] + Lhs[3][2] * Rhs[2][3] + Lhs[3][3] * Rhs[3][3] );
}

BcF32 BcMat4d::determinant()
{
	const BcMat4d& Lhs = (*this);

	const BcMat3d A = BcMat3d( BcVec3d( Lhs[1][1], Lhs[1][2], Lhs[1][3] ),
	                           BcVec3d( Lhs[2][1], Lhs[2][2], Lhs[2][3] ),
	                           BcVec3d( Lhs[3][1], Lhs[3][2], Lhs[3][3] ) );
	const BcMat3d B = BcMat3d( BcVec3d( Lhs[1][0], Lhs[1][2], Lhs[1][3] ),
	                           BcVec3d( Lhs[2][0], Lhs[2][2], Lhs[2][3] ),
	                           BcVec3d( Lhs[3][0], Lhs[3][2], Lhs[3][3] ) );
	const BcMat3d C = BcMat3d( BcVec3d( Lhs[1][0], Lhs[1][1], Lhs[1][3] ),
	                           BcVec3d( Lhs[2][0], Lhs[2][1], Lhs[2][3] ),
	                           BcVec3d( Lhs[3][0], Lhs[3][1], Lhs[3][3] ) );
	const BcMat3d D = BcMat3d( BcVec3d( Lhs[1][0], Lhs[1][1], Lhs[1][2] ),
	                           BcVec3d( Lhs[2][0], Lhs[2][1], Lhs[2][2] ),
	                           BcVec3d( Lhs[3][0], Lhs[3][1], Lhs[3][2] ) );

	return ( ( A.determinant() * Lhs[0][0] ) - 
	         ( B.determinant() * Lhs[0][1] ) + 
	         ( C.determinant() * Lhs[0][2] ) -
	         ( D.determinant() * Lhs[0][3] ) );
}

void BcMat4d::rotation( const BcVec3d& Angles )
{
	BcF32 sy, sp, sr;
	BcF32 cy, cp, cr;
	
	sy = BcSin( Angles.y() );
	sp = BcSin( Angles.x() );
	sr = BcSin( Angles.z() );
	
	cy = BcCos( Angles.y() );
	cp = BcCos( Angles.x() );
	cr = BcCos( Angles.z() );
	
	Row0_.set( cy * cr + sy * sp * sr, -cy * sr + sy * sp * cr, sy * cp, 0.0f );
	Row1_.set( sr * cp, cr * cp, -sp, 0.0f );
	Row2_.set( -sy * cr + cy * sp * sr, sr * sy + cy * sp * cr, cy * cp, 0.0f );
}

void BcMat4d::translation( const BcVec3d& Translation )
{
	translation( BcVec4d( Translation.x(), Translation.y(), Translation.z(), 1.0f ) );
}

void BcMat4d::translation( const BcVec4d& Translation )
{
	row3( Translation );
}


void BcMat4d::scale( const BcVec3d& Scale )
{
	scale( BcVec4d( Scale.x(), Scale.y(), Scale.z(), 1.0f ) );
}

void BcMat4d::scale( const BcVec4d& Scale )
{
	row0( BcVec4d( Scale.x(), 0.0f, 0.0f, 0.0f ) );
	row1( BcVec4d( 0.0f, Scale.y(), 0.0f, 0.0f ) );
	row2( BcVec4d( 0.0f, 0.0f, Scale.z(), 0.0f ) );
	row3( BcVec4d( 0.0f, 0.0f, 0.0f, Scale.w() ) );
}

void BcMat4d::inverse()
{
	const BcMat4d& Lhs = (*this);
	BcF32 Det, InvDet;

	const BcF32 Det2_01_01 = Lhs[0][0] * Lhs[1][1] - Lhs[0][1] * Lhs[1][0];
	const BcF32 Det2_01_02 = Lhs[0][0] * Lhs[1][2] - Lhs[0][2] * Lhs[1][0];
	const BcF32 Det2_01_03 = Lhs[0][0] * Lhs[1][3] - Lhs[0][3] * Lhs[1][0];
	const BcF32 Det2_01_12 = Lhs[0][1] * Lhs[1][2] - Lhs[0][2] * Lhs[1][1];
	const BcF32 Det2_01_13 = Lhs[0][1] * Lhs[1][3] - Lhs[0][3] * Lhs[1][1];
	const BcF32 Det2_01_23 = Lhs[0][2] * Lhs[1][3] - Lhs[0][3] * Lhs[1][2];

	const BcF32 Det3_201_012 = Lhs[2][0] * Det2_01_12 - Lhs[2][1] * Det2_01_02 + Lhs[2][2] * Det2_01_01;
	const BcF32 Det3_201_013 = Lhs[2][0] * Det2_01_13 - Lhs[2][1] * Det2_01_03 + Lhs[2][3] * Det2_01_01;
	const BcF32 Det3_201_023 = Lhs[2][0] * Det2_01_23 - Lhs[2][2] * Det2_01_03 + Lhs[2][3] * Det2_01_02;
	const BcF32 Det3_201_123 = Lhs[2][1] * Det2_01_23 - Lhs[2][2] * Det2_01_13 + Lhs[2][3] * Det2_01_12;

	Det = ( - Det3_201_123 * Lhs[3][0] + Det3_201_023 * Lhs[3][1] - Det3_201_013 * Lhs[3][2] + Det3_201_012 * Lhs[3][3] );

	InvDet = 1.0f / Det;

	const BcF32 Det2_03_01 = Lhs[0][0] * Lhs[3][1] - Lhs[0][1] * Lhs[3][0];
	const BcF32 Det2_03_02 = Lhs[0][0] * Lhs[3][2] - Lhs[0][2] * Lhs[3][0];
	const BcF32 Det2_03_03 = Lhs[0][0] * Lhs[3][3] - Lhs[0][3] * Lhs[3][0];
	const BcF32 Det2_03_12 = Lhs[0][1] * Lhs[3][2] - Lhs[0][2] * Lhs[3][1];
	const BcF32 Det2_03_13 = Lhs[0][1] * Lhs[3][3] - Lhs[0][3] * Lhs[3][1];
	const BcF32 Det2_03_23 = Lhs[0][2] * Lhs[3][3] - Lhs[0][3] * Lhs[3][2];

	const BcF32 Det2_13_01 = Lhs[1][0] * Lhs[3][1] - Lhs[1][1] * Lhs[3][0];
	const BcF32 Det2_13_02 = Lhs[1][0] * Lhs[3][2] - Lhs[1][2] * Lhs[3][0];
	const BcF32 Det2_13_03 = Lhs[1][0] * Lhs[3][3] - Lhs[1][3] * Lhs[3][0];
	const BcF32 Det2_13_12 = Lhs[1][1] * Lhs[3][2] - Lhs[1][2] * Lhs[3][1];
	const BcF32 Det2_13_13 = Lhs[1][1] * Lhs[3][3] - Lhs[1][3] * Lhs[3][1];
	const BcF32 Det2_13_23 = Lhs[1][2] * Lhs[3][3] - Lhs[1][3] * Lhs[3][2];

	const BcF32 Det3_203_012 = Lhs[2][0] * Det2_03_12 - Lhs[2][1] * Det2_03_02 + Lhs[2][2] * Det2_03_01;
	const BcF32 Det3_203_013 = Lhs[2][0] * Det2_03_13 - Lhs[2][1] * Det2_03_03 + Lhs[2][3] * Det2_03_01;
	const BcF32 Det3_203_023 = Lhs[2][0] * Det2_03_23 - Lhs[2][2] * Det2_03_03 + Lhs[2][3] * Det2_03_02;
	const BcF32 Det3_203_123 = Lhs[2][1] * Det2_03_23 - Lhs[2][2] * Det2_03_13 + Lhs[2][3] * Det2_03_12;

	const BcF32 Det3_213_012 = Lhs[2][0] * Det2_13_12 - Lhs[2][1] * Det2_13_02 + Lhs[2][2] * Det2_13_01;
	const BcF32 Det3_213_013 = Lhs[2][0] * Det2_13_13 - Lhs[2][1] * Det2_13_03 + Lhs[2][3] * Det2_13_01;
	const BcF32 Det3_213_023 = Lhs[2][0] * Det2_13_23 - Lhs[2][2] * Det2_13_03 + Lhs[2][3] * Det2_13_02;
	const BcF32 Det3_213_123 = Lhs[2][1] * Det2_13_23 - Lhs[2][2] * Det2_13_13 + Lhs[2][3] * Det2_13_12;

	const BcF32 Det3_301_012 = Lhs[3][0] * Det2_01_12 - Lhs[3][1] * Det2_01_02 + Lhs[3][2] * Det2_01_01;
	const BcF32 Det3_301_013 = Lhs[3][0] * Det2_01_13 - Lhs[3][1] * Det2_01_03 + Lhs[3][3] * Det2_01_01;
	const BcF32 Det3_301_023 = Lhs[3][0] * Det2_01_23 - Lhs[3][2] * Det2_01_03 + Lhs[3][3] * Det2_01_02;
	const BcF32 Det3_301_123 = Lhs[3][1] * Det2_01_23 - Lhs[3][2] * Det2_01_13 + Lhs[3][3] * Det2_01_12;

	Row0_.x( -Det3_213_123 * InvDet );
	Row1_.x(  Det3_213_023 * InvDet );
	Row2_.x( -Det3_213_013 * InvDet );
	Row3_.x(  Det3_213_012 * InvDet );

	Row0_.y(  Det3_203_123 * InvDet );
	Row1_.y( -Det3_203_023 * InvDet );
	Row2_.y(  Det3_203_013 * InvDet );
	Row3_.y( -Det3_203_012 * InvDet );

	Row0_.z(  Det3_301_123 * InvDet );
	Row1_.z( -Det3_301_023 * InvDet );
	Row2_.z(  Det3_301_013 * InvDet );
	Row3_.z( -Det3_301_012 * InvDet );

	Row0_.w( -Det3_201_123 * InvDet );
	Row1_.w(  Det3_201_023 * InvDet );
	Row2_.w( -Det3_201_013 * InvDet );
	Row3_.w(  Det3_201_012 * InvDet );
}

//////////////////////////////////////////////////////////////////////////
// lookAt
void BcMat4d::lookAt( const BcVec3d& Position, const BcVec3d& LookAt, const BcVec3d& UpVec )
{
	const BcVec3d Front = ( Position - LookAt ).normal();
	const BcVec3d Side = Front.cross( UpVec ).normal();
	const BcVec3d Up = Side.cross( Front ).normal();

	BcMat4d RotMatrix( BcVec4d( Side.x(),		Up.x(),		-Front.x(),		0.0f ),
	                   BcVec4d( Side.y(),		Up.y(),		-Front.y(),		0.0f ),
	                   BcVec4d( Side.z(),		Up.z(),		-Front.z(),		0.0f ),
	                   BcVec4d( 0.0f,			0.0f,			0.0f,		1.0f ) );


	BcMat4d TransMatrix( BcVec4d( 1.0f,			0.0f,			0.0f,		0.0f ),
	                   BcVec4d( 0.0f,			1.0f,			0.0f,		0.0f ),
	                   BcVec4d( 0.0f,			0.0f,			1.0f,		0.0f ),
	                   BcVec4d( -Position.x(), -Position.y(), -Position.z(),  1.0f ) );

	(*this) = TransMatrix * RotMatrix;
}


//////////////////////////////////////////////////////////////////////////
// orthoProjection
void BcMat4d::orthoProjection( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far )
{
	// TODO: Optimise.
	BcMat4d& Projection = (*this);
	
	Projection[0][0] = 2.0f / ( Right - Left );
	Projection[0][1] = 0.0f;
	Projection[0][2] = 0.0f;
	Projection[0][3] = 0.0f;
	
	Projection[1][0] = 0.0f;
	Projection[1][1] = 2.0f / ( Top - Bottom );
	Projection[1][2] = 0.0f;
	Projection[1][3] = 0.0f;
	
	Projection[2][0] = 0.0f;
	Projection[2][1] = 0.0f;
	Projection[2][2] = 2.0f / ( Far - Near );
	Projection[2][3] = 0.0f;
	
	Projection[3][0] = -( Right + Left ) / ( Right - Left );
	Projection[3][1] = -( Top + Bottom ) / ( Top - Bottom );
	Projection[3][2] = -( Far + Near )  / ( Far - Near );
	Projection[3][3] = 1.0f;
}

//////////////////////////////////////////////////////////////////////////
// perspProjectionHorizontal
void BcMat4d::perspProjectionHorizontal( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far )
{
	const BcF32 W = BcTan( Fov ) * Near;
	const BcF32 H = W  / Aspect;
	
    frustum( -W, W, H, -H, Near, Far );
}

//////////////////////////////////////////////////////////////////////////
// perspProjectionVertical
void BcMat4d::perspProjectionVertical( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far )
{
	const BcF32 H = BcTan( Fov ) * Near;
	const BcF32 W = H / Aspect;
	
    frustum( -W, W, H, -H, Near, Far );
}

//////////////////////////////////////////////////////////////////////////
// frustum
void BcMat4d::frustum( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far )
{
	// TODO: Optimise.
	BcMat4d& Projection = (*this);
	
	Projection[0][0] = ( 2.0f * Near ) / ( Right - Left );
	Projection[0][1] = 0.0f;
	Projection[0][2] = 0.0f;
	Projection[0][3] = 0.0f;
	
	Projection[1][0] = 0.0f;
	Projection[1][1] = ( 2.0f * Near ) / ( Bottom - Top );
	Projection[1][2] = 0.0f;
	Projection[1][3] = 0.0f;
	
	Projection[2][0] = -( Right + Left ) / ( Right - Left );
	Projection[2][1] = -( Top + Bottom ) / ( Bottom - Top );
	Projection[2][2] = ( Far + Near ) / ( Far - Near );
	Projection[2][3] = 1.0f;
	
	Projection[3][0] = 0.0f;
	Projection[3][1] = 0.0f;
	Projection[3][2] = -( 2.0f * Far * Near ) / ( Far - Near );
	Projection[3][3] = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// operator ==
BcBool BcMat4d::operator == ( const BcMat4d& Other ) const
{
	return Row0_ == Other.Row0_ &&
	       Row1_ == Other.Row1_ &&
	       Row2_ == Other.Row2_ &&
	       Row3_ == Other.Row3_;

}

//////////////////////////////////////////////////////////////////////////
// isIdentity
BcBool BcMat4d::isIdentity() const
{
	static BcMat4d Identity;
	return (*this) == Identity;
}


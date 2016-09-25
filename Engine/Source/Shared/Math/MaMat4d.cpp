/**************************************************************************
*
* File:		MaMat4d.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "Math/MaMat3d.h"
#include "Math/MaMat4d.h"

#include "Math/MaVec2d.h"
#include "Math/MaVec3d.h"

#include "Base/BcMath.h"

#include "Reflection/ReReflection.h"

REFLECTION_DEFINE_BASIC( MaMat4d );

void MaMat4d::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Row0_",		&MaMat4d::Row0_ ),
		new ReField( "Row1_",		&MaMat4d::Row1_ ),
		new ReField( "Row2_",		&MaMat4d::Row2_ ),
		new ReField( "Row3_",		&MaMat4d::Row3_ ),
	};
		
	ReRegisterClass< MaMat4d >( Fields );
}

MaMat4d MaMat4d::operator + ( const MaMat4d& Rhs )
{
	return MaMat4d( Row0_ + Rhs.Row0_,
	                Row1_ + Rhs.Row1_,
	                Row2_ + Rhs.Row2_,
	                Row3_ + Rhs.Row3_ );
}

MaMat4d MaMat4d::operator - ( const MaMat4d& Rhs )
{
	return MaMat4d( Row0_ - Rhs.Row0_,
	                Row1_ - Rhs.Row1_,
	                Row2_ - Rhs.Row2_,
	                Row3_ - Rhs.Row3_ );
}

MaMat4d MaMat4d::operator * ( BcF32 Rhs )
{
	return MaMat4d( Row0_ * Rhs,
	                Row1_ * Rhs,
	                Row2_ * Rhs,
	                Row3_ * Rhs );
}

MaMat4d MaMat4d::operator / ( BcF32 Rhs )
{
	return MaMat4d( Row0_ / Rhs,
	                Row1_ / Rhs,
	                Row2_ / Rhs,
	                Row3_ / Rhs );
}

MaMat4d MaMat4d::operator * ( const MaMat4d& Rhs ) const
{
	const MaMat4d& Lhs = (*this);

	return MaMat4d( Lhs[0][0] * Rhs[0][0] + Lhs[0][1] * Rhs[1][0] + Lhs[0][2] * Rhs[2][0] + Lhs[0][3] * Rhs[3][0],
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

BcF32 MaMat4d::determinant()
{
	const MaMat4d& Lhs = (*this);

	const MaMat3d A = MaMat3d( MaVec3d( Lhs[1][1], Lhs[1][2], Lhs[1][3] ),
	                           MaVec3d( Lhs[2][1], Lhs[2][2], Lhs[2][3] ),
	                           MaVec3d( Lhs[3][1], Lhs[3][2], Lhs[3][3] ) );
	const MaMat3d B = MaMat3d( MaVec3d( Lhs[1][0], Lhs[1][2], Lhs[1][3] ),
	                           MaVec3d( Lhs[2][0], Lhs[2][2], Lhs[2][3] ),
	                           MaVec3d( Lhs[3][0], Lhs[3][2], Lhs[3][3] ) );
	const MaMat3d C = MaMat3d( MaVec3d( Lhs[1][0], Lhs[1][1], Lhs[1][3] ),
	                           MaVec3d( Lhs[2][0], Lhs[2][1], Lhs[2][3] ),
	                           MaVec3d( Lhs[3][0], Lhs[3][1], Lhs[3][3] ) );
	const MaMat3d D = MaMat3d( MaVec3d( Lhs[1][0], Lhs[1][1], Lhs[1][2] ),
	                           MaVec3d( Lhs[2][0], Lhs[2][1], Lhs[2][2] ),
	                           MaVec3d( Lhs[3][0], Lhs[3][1], Lhs[3][2] ) );

	return ( ( A.determinant() * Lhs[0][0] ) - 
	         ( B.determinant() * Lhs[0][1] ) + 
	         ( C.determinant() * Lhs[0][2] ) -
	         ( D.determinant() * Lhs[0][3] ) );
}

void MaMat4d::rotation( const MaVec3d& Angles )
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

void MaMat4d::translation( const MaVec3d& Translation )
{
	translation( MaVec4d( Translation.x(), Translation.y(), Translation.z(), 1.0f ) );
}

void MaMat4d::translation( const MaVec4d& Translation )
{
	row3( Translation );
}


void MaMat4d::scale( const MaVec3d& Scale )
{
	scale( MaVec4d( Scale.x(), Scale.y(), Scale.z(), 1.0f ) );
}

void MaMat4d::scale( const MaVec4d& Scale )
{
	row0( MaVec4d( Scale.x(), 0.0f, 0.0f, 0.0f ) );
	row1( MaVec4d( 0.0f, Scale.y(), 0.0f, 0.0f ) );
	row2( MaVec4d( 0.0f, 0.0f, Scale.z(), 0.0f ) );
	row3( MaVec4d( 0.0f, 0.0f, 0.0f, Scale.w() ) );
}

void MaMat4d::inverse()
{
	const MaMat4d& Lhs = (*this);
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
void MaMat4d::lookAt( const MaVec3d& Position, const MaVec3d& LookAt, const MaVec3d& UpVec )
{
	const MaVec3d Front = ( Position - LookAt ).normal();
	const MaVec3d Side = Front.cross( UpVec ).normal();
	const MaVec3d Up = Side.cross( Front ).normal();

	MaMat4d RotMatrix( MaVec4d( Side.x(),		Up.x(),		-Front.x(),		0.0f ),
	                   MaVec4d( Side.y(),		Up.y(),		-Front.y(),		0.0f ),
	                   MaVec4d( Side.z(),		Up.z(),		-Front.z(),		0.0f ),
	                   MaVec4d( 0.0f,			0.0f,			0.0f,		1.0f ) );


	MaMat4d TransMatrix( MaVec4d( 1.0f,			0.0f,			0.0f,		0.0f ),
	                   MaVec4d( 0.0f,			1.0f,			0.0f,		0.0f ),
	                   MaVec4d( 0.0f,			0.0f,			1.0f,		0.0f ),
	                   MaVec4d( -Position.x(), -Position.y(), -Position.z(),  1.0f ) );

	(*this) = TransMatrix * RotMatrix;
}


//////////////////////////////////////////////////////////////////////////
// orthoProjection
void MaMat4d::orthoProjection( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far )
{
	// TODO: Optimise.
	MaMat4d& Projection = (*this);
	
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
void MaMat4d::perspProjectionHorizontal( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far )
{
	const BcF32 W = BcTan( Fov ) * Near;
	const BcF32 H = W  / Aspect;
	
    frustum( -W, W, H, -H, Near, Far );
}

//////////////////////////////////////////////////////////////////////////
// perspProjectionVertical
void MaMat4d::perspProjectionVertical( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far )
{
	const BcF32 H = BcTan( Fov ) * Near;
	const BcF32 W = H / Aspect;
	
    frustum( -W, W, H, -H, Near, Far );
}

//////////////////////////////////////////////////////////////////////////
// frustum
void MaMat4d::frustum( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far )
{
	// TODO: Optimise.
	MaMat4d& Projection = (*this);
	
	Projection[0][0] = ( 2.0f * Near ) / ( Right - Left );
	Projection[0][1] = 0.0f;
	Projection[0][2] = 0.0f;
	Projection[0][3] = 0.0f;
	
	Projection[1][0] = 0.0f;
	Projection[1][1] = ( 2.0f * Near ) / ( Bottom - Top );
	Projection[1][2] = 0.0f;
	Projection[1][3] = 0.0f;
	
	Projection[2][0] = 0.0f;
	Projection[2][1] = 0.0f;
	Projection[2][2] = ( Far + Near ) / ( Far - Near );
	Projection[2][3] = 1.0f;
	
	Projection[3][0] = 0.0f;
	Projection[3][1] = 0.0f;
	Projection[3][2] = -( 2.0f * Far * Near ) / ( Far - Near );
	Projection[3][3] = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// operator ==
BcBool MaMat4d::operator == ( const MaMat4d& Other ) const
{
	return Row0_ == Other.Row0_ &&
	       Row1_ == Other.Row1_ &&
	       Row2_ == Other.Row2_ &&
	       Row3_ == Other.Row3_;

}

//////////////////////////////////////////////////////////////////////////
// isIdentity
BcBool MaMat4d::isIdentity() const
{
	static MaMat4d Identity;
	return (*this) == Identity;
}

MaVec2d operator * ( const MaVec2d& Lhs, const MaMat4d& Rhs )
{
	return MaVec2d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Rhs[3][0],
				    Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Rhs[3][1] );
}

MaVec3d operator * ( const MaVec3d& Lhs, const MaMat4d& Rhs )
{
	return MaVec3d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Lhs.z() * Rhs[2][0] + Rhs[3][0],
		            Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Lhs.z() * Rhs[2][1] + Rhs[3][1],
		            Lhs.x() * Rhs[0][2] + Lhs.y() * Rhs[1][2] + Lhs.z() * Rhs[2][2] + Rhs[3][2] );
}

MaVec4d operator * ( const MaVec4d& Lhs, const MaMat4d& Rhs )
{
	return MaVec4d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Lhs.z() * Rhs[2][0] + Lhs.w() * Rhs[3][0],
	                Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Lhs.z() * Rhs[2][1] + Lhs.w() * Rhs[3][1],
	                Lhs.x() * Rhs[0][2] + Lhs.y() * Rhs[1][2] + Lhs.z() * Rhs[2][2] + Lhs.w() * Rhs[3][2],
	                Lhs.x() * Rhs[0][3] + Lhs.y() * Rhs[1][3] + Lhs.z() * Rhs[2][3] + Lhs.w() * Rhs[3][3] );
}

MaVec3d MaMat4d::translation() const
{
	return MaVec3d( Row3_.x(), Row3_.y(), Row3_.z() );
}
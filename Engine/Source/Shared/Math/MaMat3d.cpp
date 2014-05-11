/**************************************************************************
*
* File:		MaMat3d.cpp
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

REFLECTION_DEFINE_BASIC( MaMat3d );

void MaMat3d::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Row0_",		&MaMat3d::Row0_ ),
		ReField( "Row1_",		&MaMat3d::Row1_ ),
		ReField( "Row2_",		&MaMat3d::Row2_ ),
	};
		
	ReRegisterClass< MaMat3d >( Fields );
}

MaMat3d::MaMat3d( const MaVec3d& Row0,
                  const MaVec3d& Row1,
                  const MaVec3d& Row2 )
{
	Row0_ = Row0;
	Row1_ = Row1;
	Row2_ = Row2;
}



MaMat3d::MaMat3d( BcF32 I00,
			      BcF32 I01,
			      BcF32 I02,
			      BcF32 I10,
			      BcF32 I11,
			      BcF32 I12,
			      BcF32 I20,
			      BcF32 I21,
			      BcF32 I22 )
{
	Row0_.set( I00, I01, I02 );
	Row1_.set( I10, I11, I12 );
	Row2_.set( I20, I21, I22 );
}

MaMat3d MaMat3d::operator + ( const MaMat3d& Rhs )
{
	return MaMat3d( Row0_ + Rhs.Row0_,
	                Row1_ + Rhs.Row1_,
	                Row2_ + Rhs.Row2_);
}

MaMat3d MaMat3d::operator - ( const MaMat3d& Rhs )
{
	return MaMat3d( Row0_ - Rhs.Row0_,
	                Row1_ - Rhs.Row1_,
	                Row2_ - Rhs.Row2_ );
}

MaMat3d MaMat3d::operator * ( BcF32 Rhs )
{
	return MaMat3d( Row0_ * Rhs,
	                Row1_ * Rhs,
	                Row2_ * Rhs );
}

MaMat3d MaMat3d::operator / ( BcF32 Rhs )
{
	return MaMat3d( Row0_ / Rhs,
	                Row1_ / Rhs,
	                Row2_ / Rhs );
}

MaMat3d MaMat3d::operator * ( const MaMat3d& Rhs )
{
	const MaMat3d& Lhs = (*this);

	return MaMat3d( Lhs[0][0] * Rhs[0][0] + Lhs[0][1] * Rhs[1][0] + Lhs[0][2] * Rhs[2][0],
	                Lhs[0][0] * Rhs[0][1] + Lhs[0][1] * Rhs[1][1] + Lhs[0][2] * Rhs[2][1],
	                Lhs[0][0] * Rhs[0][2] + Lhs[0][1] * Rhs[1][2] + Lhs[0][2] * Rhs[2][2],
	                Lhs[1][0] * Rhs[0][0] + Lhs[1][1] * Rhs[1][0] + Lhs[1][2] * Rhs[2][0],
	                Lhs[1][0] * Rhs[0][1] + Lhs[1][1] * Rhs[1][1] + Lhs[1][2] * Rhs[2][1],
	                Lhs[1][0] * Rhs[0][2] + Lhs[1][1] * Rhs[1][2] + Lhs[1][2] * Rhs[2][2],
	                Lhs[2][0] * Rhs[0][0] + Lhs[2][1] * Rhs[1][0] + Lhs[2][2] * Rhs[2][0],
	                Lhs[2][0] * Rhs[0][1] + Lhs[2][1] * Rhs[1][1] + Lhs[2][2] * Rhs[2][1],
	                Lhs[2][0] * Rhs[0][2] + Lhs[2][1] * Rhs[1][2] + Lhs[2][2] * Rhs[2][2] );
}

BcF32 MaMat3d::determinant() const
{
	const MaMat3d& Lhs = (*this);

	return ( Lhs[0][0] * Lhs[1][1] * Lhs[2][2] ) - 
	       ( Lhs[0][0] * Lhs[1][2] * Lhs[2][1] ) +
	       ( Lhs[0][1] * Lhs[1][2] * Lhs[2][0] ) -
	       ( Lhs[0][1] * Lhs[1][0] * Lhs[2][2] ) +
	       ( Lhs[0][2] * Lhs[1][0] * Lhs[2][1] ) - 
	       ( Lhs[0][2] * Lhs[1][1] * Lhs[2][0] );
}

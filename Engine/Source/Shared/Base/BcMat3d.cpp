/**************************************************************************
*
* File:		BcMat3d.cpp
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

BcMat3d::BcMat3d( const BcVec3d& Row0,
                  const BcVec3d& Row1,
                  const BcVec3d& Row2 )
{
	Row0_ = Row0;
	Row1_ = Row1;
	Row2_ = Row2;
}



BcMat3d::BcMat3d( BcReal I00,
			      BcReal I01,
			      BcReal I02,
			      BcReal I10,
			      BcReal I11,
			      BcReal I12,
			      BcReal I20,
			      BcReal I21,
			      BcReal I22 )
{
	Row0_.set( I00, I01, I02 );
	Row1_.set( I10, I11, I12 );
	Row2_.set( I20, I21, I22 );
}

BcMat3d BcMat3d::operator + ( const BcMat3d& Rhs )
{
	return BcMat3d( Row0_ + Rhs.Row0_,
	                Row1_ + Rhs.Row1_,
	                Row2_ + Rhs.Row2_);
}

BcMat3d BcMat3d::operator - ( const BcMat3d& Rhs )
{
	return BcMat3d( Row0_ - Rhs.Row0_,
	                Row1_ - Rhs.Row1_,
	                Row2_ - Rhs.Row2_ );
}

BcMat3d BcMat3d::operator * ( BcReal Rhs )
{
	return BcMat3d( Row0_ * Rhs,
	                Row1_ * Rhs,
	                Row2_ * Rhs );
}

BcMat3d BcMat3d::operator / ( BcReal Rhs )
{
	return BcMat3d( Row0_ / Rhs,
	                Row1_ / Rhs,
	                Row2_ / Rhs );
}

BcMat3d BcMat3d::operator * ( const BcMat3d& Rhs )
{
	const BcMat3d& Lhs = (*this);

	return BcMat3d( Lhs[0][0] * Rhs[0][0] + Lhs[0][1] * Rhs[1][0] + Lhs[0][2] * Rhs[2][0],
	                Lhs[0][0] * Rhs[0][1] + Lhs[0][1] * Rhs[1][1] + Lhs[0][2] * Rhs[2][1],
	                Lhs[0][0] * Rhs[0][2] + Lhs[0][1] * Rhs[1][2] + Lhs[0][2] * Rhs[2][2],
	                Lhs[1][0] * Rhs[0][0] + Lhs[1][1] * Rhs[1][0] + Lhs[1][2] * Rhs[2][0],
	                Lhs[1][0] * Rhs[0][1] + Lhs[1][1] * Rhs[1][1] + Lhs[1][2] * Rhs[2][1],
	                Lhs[1][0] * Rhs[0][2] + Lhs[1][1] * Rhs[1][2] + Lhs[1][2] * Rhs[2][2],
	                Lhs[2][0] * Rhs[0][0] + Lhs[2][1] * Rhs[1][0] + Lhs[2][2] * Rhs[2][0],
	                Lhs[2][0] * Rhs[0][1] + Lhs[2][1] * Rhs[1][1] + Lhs[2][2] * Rhs[2][1],
	                Lhs[2][0] * Rhs[0][2] + Lhs[2][1] * Rhs[1][2] + Lhs[2][2] * Rhs[2][2] );
}

BcReal BcMat3d::determinant() const
{
	const BcMat3d& Lhs = (*this);

	return ( Lhs[0][0] * Lhs[1][1] * Lhs[2][2] ) - 
	       ( Lhs[0][0] * Lhs[1][2] * Lhs[2][1] ) +
	       ( Lhs[0][1] * Lhs[1][2] * Lhs[2][0] ) -
	       ( Lhs[0][1] * Lhs[1][0] * Lhs[2][2] ) +
	       ( Lhs[0][2] * Lhs[1][0] * Lhs[2][1] ) - 
	       ( Lhs[0][2] * Lhs[1][1] * Lhs[2][0] );
}

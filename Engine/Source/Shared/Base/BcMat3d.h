/**************************************************************************
*
* File:		BcMat3d.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Row major 3x3 matrix
*		
*
*
* 
**************************************************************************/

#ifndef __BCMAT3D_H__
#define __BCMAT3D_H__

#include "Base/BcVectors.h"

//////////////////////////////////////////////////////////////////////////
// BcMat3d
class BcMat3d
{
public:

	BcMat3d(){}	
	BcMat3d( const BcVec3d&,
			 const BcVec3d&,
			 const BcVec3d& );

	BcMat3d( BcReal I00,
			 BcReal I01,
			 BcReal I02,
			 BcReal I10,
			 BcReal I11,
			 BcReal I12,
			 BcReal I20,
			 BcReal I21,
			 BcReal I22 );

	// Accessor
	BcReal*			operator [] ( BcU32 i );
	const BcReal*	operator [] ( BcU32 i ) const;

	const BcVec3d&	row0() const;
	const BcVec3d&	row1() const;
	const BcVec3d&	row2() const;

	void row0( const BcVec3d& Row );
	void row1( const BcVec3d& Row );
	void row2( const BcVec3d& Row );

	// Arithmetic
	BcMat3d		operator + ( const BcMat3d& rhs );
	BcMat3d		operator - ( const BcMat3d& rhs );
	BcMat3d		operator * ( BcReal rhs );
	BcMat3d		operator / ( BcReal rhs );
	BcMat3d		operator * ( const BcMat3d& rhs );

	void		identity();
	void		transpose();

	BcReal		determinant() const;

private:
	BcVec3d Row0_;
	BcVec3d Row1_;
	BcVec3d Row2_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline BcReal* BcMat3d::operator [] ( BcU32 i )
{
	return reinterpret_cast< BcReal* >( &Row0_ ) + ( i * 3 );
}

inline const BcReal* BcMat3d::operator [] ( BcU32 i ) const
{
	return reinterpret_cast< const BcReal* >( &Row0_ ) + ( i * 3 );
}

inline const BcVec3d& BcMat3d::row0() const
{
	return Row0_;
}

inline const BcVec3d& BcMat3d::row1() const
{
	return Row1_;
}

inline const BcVec3d& BcMat3d::row2() const
{
	return Row2_;
}

inline void BcMat3d::row0( const BcVec3d& Row0 )
{
	Row0_ = Row0;
}

inline void BcMat3d::row1( const BcVec3d& Row1 )
{
	Row1_ = Row1;
}

inline void BcMat3d::row2( const BcVec3d& Row2 )
{
	Row2_ = Row2;
}

inline void BcMat3d::identity()
{
	Row0_.set( 1.0f, 0.0f, 0.0f );
	Row1_.set( 0.0f, 1.0f, 0.0f );
	Row2_.set( 0.0f, 0.0f, 1.0f );
}

inline void BcMat3d::transpose()
{
	const BcMat3d& Lhs = (*this);

	(*this) = BcMat3d( BcVec3d( Lhs[0][0], Lhs[1][0], Lhs[2][0] ),
	                   BcVec3d( Lhs[0][1], Lhs[1][1], Lhs[2][1] ),
	                   BcVec3d( Lhs[0][2], Lhs[1][2], Lhs[2][2] ) );
}

//
inline BcVec3d operator * ( const BcVec3d& Lhs, const BcMat3d& Rhs )
{
	return BcVec3d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Lhs.z() * Rhs[2][0],
	                Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Lhs.z() * Rhs[2][1],
	                Lhs.x() * Rhs[0][2] + Lhs.y() * Rhs[1][2] + Lhs.z() * Rhs[2][2] );
}

#endif


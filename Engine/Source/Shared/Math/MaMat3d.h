/**************************************************************************
*
* File:		MaMat3d.h
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

#include "Math/MaVec3d.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// MaMat3d
class MaMat3d
{
public:
	REFLECTION_DECLARE_BASIC( MaMat3d );

public:

	MaMat3d(){}	
	MaMat3d( const MaVec3d&,
			 const MaVec3d&,
			 const MaVec3d& );

	MaMat3d( BcF32 I00,
			 BcF32 I01,
			 BcF32 I02,
			 BcF32 I10,
			 BcF32 I11,
			 BcF32 I12,
			 BcF32 I20,
			 BcF32 I21,
			 BcF32 I22 );

	// Accessor
	BcF32*			operator [] ( BcU32 i );
	const BcF32*	operator [] ( BcU32 i ) const;

	const MaVec3d&	row0() const;
	const MaVec3d&	row1() const;
	const MaVec3d&	row2() const;

	void row0( const MaVec3d& Row );
	void row1( const MaVec3d& Row );
	void row2( const MaVec3d& Row );

	// Arithmetic
	MaMat3d		operator + ( const MaMat3d& rhs );
	MaMat3d		operator - ( const MaMat3d& rhs );
	MaMat3d		operator * ( BcF32 rhs );
	MaMat3d		operator / ( BcF32 rhs );
	MaMat3d		operator * ( const MaMat3d& rhs );

	void		identity();
	void		transpose();

	BcF32		determinant() const;

private:
	MaVec3d Row0_;
	MaVec3d Row1_;
	MaVec3d Row2_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline BcF32* MaMat3d::operator [] ( BcU32 i )
{
	return reinterpret_cast< BcF32* >( &Row0_ ) + ( i * 3 );
}

inline const BcF32* MaMat3d::operator [] ( BcU32 i ) const
{
	return reinterpret_cast< const BcF32* >( &Row0_ ) + ( i * 3 );
}

inline const MaVec3d& MaMat3d::row0() const
{
	return Row0_;
}

inline const MaVec3d& MaMat3d::row1() const
{
	return Row1_;
}

inline const MaVec3d& MaMat3d::row2() const
{
	return Row2_;
}

inline void MaMat3d::row0( const MaVec3d& Row0 )
{
	Row0_ = Row0;
}

inline void MaMat3d::row1( const MaVec3d& Row1 )
{
	Row1_ = Row1;
}

inline void MaMat3d::row2( const MaVec3d& Row2 )
{
	Row2_ = Row2;
}

inline void MaMat3d::identity()
{
	Row0_.set( 1.0f, 0.0f, 0.0f );
	Row1_.set( 0.0f, 1.0f, 0.0f );
	Row2_.set( 0.0f, 0.0f, 1.0f );
}

inline void MaMat3d::transpose()
{
	const MaMat3d& Lhs = (*this);

	(*this) = MaMat3d( MaVec3d( Lhs[0][0], Lhs[1][0], Lhs[2][0] ),
	                   MaVec3d( Lhs[0][1], Lhs[1][1], Lhs[2][1] ),
	                   MaVec3d( Lhs[0][2], Lhs[1][2], Lhs[2][2] ) );
}

//
inline MaVec3d operator * ( const MaVec3d& Lhs, const MaMat3d& Rhs )
{
	return MaVec3d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Lhs.z() * Rhs[2][0],
	                Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Lhs.z() * Rhs[2][1],
	                Lhs.x() * Rhs[0][2] + Lhs.y() * Rhs[1][2] + Lhs.z() * Rhs[2][2] );
}

#endif


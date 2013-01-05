/**************************************************************************
*
* File:		BcMat4d.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Row major 4x4 matrix
*		
*
*
* 
**************************************************************************/

#ifndef __BCMAT4D_H__
#define __BCMAT4D_H__

#include "Base/BcVectors.h"

//////////////////////////////////////////////////////////////////////////
// BcMat4d
class BcMat4d
{
private:
	BcVec4d Row0_;
	BcVec4d Row1_;
	BcVec4d Row2_;
	BcVec4d Row3_;

public:

	BcMat4d()
	{
		identity();
	}
	BcMat4d( const BcVec4d&,
			 const BcVec4d&,
			 const BcVec4d&,
			 const BcVec4d& );

	BcMat4d( BcF32 I00,
	         BcF32 I01,
	         BcF32 I02,
	         BcF32 I03,
	         BcF32 I10,
	         BcF32 I11,
	         BcF32 I12,
	         BcF32 I13,
	         BcF32 I20,
	         BcF32 I21,
	         BcF32 I22,
	         BcF32 I23,
	         BcF32 I30,
	         BcF32 I31,
	         BcF32 I32,
	         BcF32 I33 );

	// Accessor
	BcF32*			operator [] ( BcU32 i );
	const BcF32*	operator [] ( BcU32 i ) const;

	const BcVec4d&	row0() const;
	const BcVec4d&	row1() const;
	const BcVec4d&	row2() const;
	const BcVec4d&	row3() const;

	void row0( const BcVec4d& Row );
	void row1( const BcVec4d& Row );
	void row2( const BcVec4d& Row );
	void row3( const BcVec4d& Row );

	BcVec4d	col0() const;
	BcVec4d	col1() const;
	BcVec4d	col2() const;
	BcVec4d	col3() const;

	void col0( const BcVec4d& Col );
	void col1( const BcVec4d& Col );
	void col2( const BcVec4d& Col );
	void col3( const BcVec4d& Col );

	// Arithmetic
	BcMat4d			operator + (const BcMat4d& rhs);
	BcMat4d			operator - (const BcMat4d& rhs);
	BcMat4d			operator * (BcF32 rhs);
	BcMat4d			operator / (BcF32 rhs);
	BcMat4d			operator * (const BcMat4d& rhs)  const;

	void			identity();
	BcMat4d			transposed() const;
	void			transpose();

	void			rotation( const BcVec3d& );

	void			translation( const BcVec3d& );
	void			translation( const BcVec4d& );
	BcVec3d			translation() const;

	void			scale( const BcVec3d& );
	void			scale( const BcVec4d& );

	BcF32			determinant();
	void			inverse();

	void			lookAt( const BcVec3d& Position, const BcVec3d& LookAt, const BcVec3d& UpVec );
	void			orthoProjection( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far );
	void			perspProjectionHorizontal( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far );
	void			perspProjectionVertical( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far );
	void			frustum( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far );

	BcBool			operator == ( const BcMat4d& Other ) const;

	BcBool			isIdentity() const;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcMat4d::BcMat4d( const BcVec4d& Row0,
                         const BcVec4d& Row1,
                         const BcVec4d& Row2,
                         const BcVec4d& Row3 )
{
	Row0_ = Row0;
	Row1_ = Row1;
	Row2_ = Row2;
	Row3_ = Row3;
}

inline BcMat4d::BcMat4d( BcF32 I00,
	                     BcF32 I01,
	                     BcF32 I02,
	                     BcF32 I03,
	                     BcF32 I10,
	                     BcF32 I11,
	                     BcF32 I12,
	                     BcF32 I13,
	                     BcF32 I20,
	                     BcF32 I21,
	                     BcF32 I22,
	                     BcF32 I23,
	                     BcF32 I30,
	                     BcF32 I31,
	                     BcF32 I32,
	                     BcF32 I33 )
{
	Row0_.set( I00, I01, I02, I03 );
	Row1_.set( I10, I11, I12, I13 );
	Row2_.set( I20, I21, I22, I23 );
	Row3_.set( I30, I31, I32, I33 );
}

inline BcF32* BcMat4d::operator [] ( BcU32 i )
{
	return reinterpret_cast< BcF32* >( &Row0_ ) + ( i * 4 );
}

inline const BcF32* BcMat4d::operator [] ( BcU32 i ) const
{
	return reinterpret_cast< const BcF32* >( &Row0_ ) + ( i * 4 );
}

inline const BcVec4d& BcMat4d::row0() const
{
	return Row0_;
}

inline const BcVec4d& BcMat4d::row1() const
{
	return Row1_;
}

inline const BcVec4d& BcMat4d::row2() const
{
	return Row2_;
}

inline const BcVec4d& BcMat4d::row3() const
{
	return Row3_;
}

inline void BcMat4d::row0( const BcVec4d& Row0 )
{
	Row0_ = Row0;
}

inline void BcMat4d::row1( const BcVec4d& Row1 )
{
	Row1_ = Row1;
}

inline void BcMat4d::row2( const BcVec4d& Row2 )
{
	Row2_ = Row2;
}

inline void BcMat4d::row3( const BcVec4d& Row3 )
{
	Row3_ = Row3;
}

inline BcVec4d BcMat4d::col0() const
{
	return BcVec4d( Row0_.x(), Row1_.x(), Row2_.x(), Row3_.x() );
}

inline BcVec4d BcMat4d::col1() const
{
	return BcVec4d( Row0_.y(), Row1_.y(), Row2_.y(), Row3_.y() );
}

inline BcVec4d BcMat4d::col2() const
{
	return BcVec4d( Row0_.z(), Row1_.z(), Row2_.z(), Row3_.z() );
}

inline BcVec4d BcMat4d::col3() const
{
	return BcVec4d( Row0_.w(), Row1_.w(), Row2_.w(), Row3_.w() );
}

inline void BcMat4d::col0( const BcVec4d& Col )
{
	Row0_.x( Col.x() );
	Row1_.x( Col.y() );
	Row2_.x( Col.z() );
	Row3_.x( Col.w() );
}

inline void BcMat4d::col1( const BcVec4d& Col )
{
	Row0_.y( Col.x() );
	Row1_.y( Col.y() );
	Row2_.y( Col.z() );
	Row3_.y( Col.w() );
}

inline void BcMat4d::col2( const BcVec4d& Col )
{
	Row0_.z( Col.x() );
	Row1_.z( Col.y() );
	Row2_.z( Col.z() );
	Row3_.z( Col.w() );
}

inline void BcMat4d::col3( const BcVec4d& Col )
{
	Row0_.w( Col.x() );
	Row1_.w( Col.y() );
	Row2_.w( Col.z() );
	Row3_.w( Col.w() );
}

inline void BcMat4d::identity()
{
	Row0_.set( 1.0f, 0.0f, 0.0f, 0.0f );
	Row1_.set( 0.0f, 1.0f, 0.0f, 0.0f );
	Row2_.set( 0.0f, 0.0f, 1.0f, 0.0f );
	Row3_.set( 0.0f, 0.0f, 0.0f, 1.0f );
}

inline BcMat4d BcMat4d::transposed() const
{
	return BcMat4d( col0(), col1(), col2(), col3() );
}

inline void BcMat4d::transpose()
{
	(*this) = transposed();
}

inline BcVec2d operator * ( const BcVec2d& Lhs, const BcMat4d& Rhs )
{
	return BcVec2d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Rhs[3][0],
				   Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Rhs[3][1] );
}

inline BcVec3d operator * ( const BcVec3d& Lhs, const BcMat4d& Rhs )
{
	return BcVec3d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Lhs.z() * Rhs[2][0] + Rhs[3][0],
		            Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Lhs.z() * Rhs[2][1] + Rhs[3][1],
		            Lhs.x() * Rhs[0][2] + Lhs.y() * Rhs[1][2] + Lhs.z() * Rhs[2][2] + Rhs[3][2] );
}

inline BcVec4d operator * ( const BcVec4d& Lhs, const BcMat4d& Rhs )
{
	return BcVec4d( Lhs.x() * Rhs[0][0] + Lhs.y() * Rhs[1][0] + Lhs.z() * Rhs[2][0] + Lhs.w() * Rhs[3][0],
	                Lhs.x() * Rhs[0][1] + Lhs.y() * Rhs[1][1] + Lhs.z() * Rhs[2][1] + Lhs.w() * Rhs[3][1],
	                Lhs.x() * Rhs[0][2] + Lhs.y() * Rhs[1][2] + Lhs.z() * Rhs[2][2] + Lhs.w() * Rhs[3][2],
	                Lhs.x() * Rhs[0][3] + Lhs.y() * Rhs[1][3] + Lhs.z() * Rhs[2][3] + Lhs.w() * Rhs[3][3] );
}

inline BcVec3d BcMat4d::translation() const
{
	return BcVec3d( Row3_.x(), Row3_.y(), Row3_.z() );
}


#endif

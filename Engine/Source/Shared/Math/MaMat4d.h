/**************************************************************************
*
* File:		MaMat4d.h
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

#include "Math/MaVec2d.h"
#include "Math/MaVec3d.h"
#include "Math/MaVec4d.h"

//////////////////////////////////////////////////////////////////////////
// MaMat4d
class MaMat4d
{
private:
	MaVec4d Row0_;
	MaVec4d Row1_;
	MaVec4d Row2_;
	MaVec4d Row3_;

public:

	MaMat4d()
	{
		identity();
	}
	MaMat4d( const MaVec4d&,
			 const MaVec4d&,
			 const MaVec4d&,
			 const MaVec4d& );

	MaMat4d( BcF32 I00,
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

	const MaVec4d&	row0() const;
	const MaVec4d&	row1() const;
	const MaVec4d&	row2() const;
	const MaVec4d&	row3() const;

	void row0( const MaVec4d& Row );
	void row1( const MaVec4d& Row );
	void row2( const MaVec4d& Row );
	void row3( const MaVec4d& Row );

	MaVec4d	col0() const;
	MaVec4d	col1() const;
	MaVec4d	col2() const;
	MaVec4d	col3() const;

	void col0( const MaVec4d& Col );
	void col1( const MaVec4d& Col );
	void col2( const MaVec4d& Col );
	void col3( const MaVec4d& Col );

	// Arithmetic
	MaMat4d			operator + (const MaMat4d& rhs);
	MaMat4d			operator - (const MaMat4d& rhs);
	MaMat4d			operator * (BcF32 rhs);
	MaMat4d			operator / (BcF32 rhs);
	MaMat4d			operator * (const MaMat4d& rhs)  const;

	void			identity();
	MaMat4d			transposed() const;
	void			transpose();

	void			rotation( const MaVec3d& );

	void			translation( const MaVec3d& );
	void			translation( const MaVec4d& );
	MaVec3d			translation() const;

	void			scale( const MaVec3d& );
	void			scale( const MaVec4d& );

	BcF32			determinant();
	void			inverse();

	void			lookAt( const MaVec3d& Position, const MaVec3d& LookAt, const MaVec3d& UpVec );
	void			orthoProjection( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far );
	void			perspProjectionHorizontal( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far );
	void			perspProjectionVertical( BcF32 Fov, BcF32 Aspect, BcF32 Near, BcF32 Far );
	void			frustum( BcF32 Left, BcF32 Right, BcF32 Bottom, BcF32 Top, BcF32 Near, BcF32 Far );

	BcBool			operator == ( const MaMat4d& Other ) const;

	BcBool			isIdentity() const;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline MaMat4d::MaMat4d( const MaVec4d& Row0,
                         const MaVec4d& Row1,
                         const MaVec4d& Row2,
                         const MaVec4d& Row3 )
{
	Row0_ = Row0;
	Row1_ = Row1;
	Row2_ = Row2;
	Row3_ = Row3;
}

inline MaMat4d::MaMat4d( BcF32 I00,
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

inline BcF32* MaMat4d::operator [] ( BcU32 i )
{
	return reinterpret_cast< BcF32* >( &Row0_ ) + ( i * 4 );
}

inline const BcF32* MaMat4d::operator [] ( BcU32 i ) const
{
	return reinterpret_cast< const BcF32* >( &Row0_ ) + ( i * 4 );
}

inline const MaVec4d& MaMat4d::row0() const
{
	return Row0_;
}

inline const MaVec4d& MaMat4d::row1() const
{
	return Row1_;
}

inline const MaVec4d& MaMat4d::row2() const
{
	return Row2_;
}

inline const MaVec4d& MaMat4d::row3() const
{
	return Row3_;
}

inline void MaMat4d::row0( const MaVec4d& Row0 )
{
	Row0_ = Row0;
}

inline void MaMat4d::row1( const MaVec4d& Row1 )
{
	Row1_ = Row1;
}

inline void MaMat4d::row2( const MaVec4d& Row2 )
{
	Row2_ = Row2;
}

inline void MaMat4d::row3( const MaVec4d& Row3 )
{
	Row3_ = Row3;
}

inline MaVec4d MaMat4d::col0() const
{
	return MaVec4d( Row0_.x(), Row1_.x(), Row2_.x(), Row3_.x() );
}

inline MaVec4d MaMat4d::col1() const
{
	return MaVec4d( Row0_.y(), Row1_.y(), Row2_.y(), Row3_.y() );
}

inline MaVec4d MaMat4d::col2() const
{
	return MaVec4d( Row0_.z(), Row1_.z(), Row2_.z(), Row3_.z() );
}

inline MaVec4d MaMat4d::col3() const
{
	return MaVec4d( Row0_.w(), Row1_.w(), Row2_.w(), Row3_.w() );
}

inline void MaMat4d::col0( const MaVec4d& Col )
{
	Row0_.x( Col.x() );
	Row1_.x( Col.y() );
	Row2_.x( Col.z() );
	Row3_.x( Col.w() );
}

inline void MaMat4d::col1( const MaVec4d& Col )
{
	Row0_.y( Col.x() );
	Row1_.y( Col.y() );
	Row2_.y( Col.z() );
	Row3_.y( Col.w() );
}

inline void MaMat4d::col2( const MaVec4d& Col )
{
	Row0_.z( Col.x() );
	Row1_.z( Col.y() );
	Row2_.z( Col.z() );
	Row3_.z( Col.w() );
}

inline void MaMat4d::col3( const MaVec4d& Col )
{
	Row0_.w( Col.x() );
	Row1_.w( Col.y() );
	Row2_.w( Col.z() );
	Row3_.w( Col.w() );
}

inline void MaMat4d::identity()
{
	Row0_.set( 1.0f, 0.0f, 0.0f, 0.0f );
	Row1_.set( 0.0f, 1.0f, 0.0f, 0.0f );
	Row2_.set( 0.0f, 0.0f, 1.0f, 0.0f );
	Row3_.set( 0.0f, 0.0f, 0.0f, 1.0f );
}

inline MaMat4d MaMat4d::transposed() const
{
	return MaMat4d( col0(), col1(), col2(), col3() );
}

inline void MaMat4d::transpose()
{
	(*this) = transposed();
}

MaVec2d operator * ( const MaVec2d& Lhs, const MaMat4d& Rhs );

MaVec3d operator * ( const MaVec3d& Lhs, const MaMat4d& Rhs );

MaVec4d operator * ( const MaVec4d& Lhs, const MaMat4d& Rhs );


#endif

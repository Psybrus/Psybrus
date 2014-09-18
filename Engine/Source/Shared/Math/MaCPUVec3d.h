/**************************************************************************
*
* File:		MaCPUVec3d.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __BCCPUVEC3D_H__
#define __BCCPUVEC3D_H__

#include "Math/MaCPUVecQuad.h"
#include "Reflection/ReReflection.h"
#include "MaVec2d.h"
#define DECLARE_SWIZZLE2( T, X, Y ) inline T X ## Y() const { return T( X(), Y() ); }
//////////////////////////////////////////////////////////////////////////
// MaCPUVec3d
class MaCPUVec3d: public MaCPUVecQuad
{
public:
	REFLECTION_DECLARE_BASIC( MaCPUVec3d );

public:
	// Ctors
	MaCPUVec3d();
	MaCPUVec3d( BcF32 X, BcF32 Y, BcF32 Z );
	MaCPUVec3d( const class MaCPUVec2d& Rhs, BcF32 Z );
	MaCPUVec3d( const BcChar* pString );

	void			set( BcF32 X, BcF32 Y, BcF32 Z );

	// Accessors
	BcForceInline BcF32			x() const { return X_; }
	BcForceInline BcF32			y() const { return Y_; }
	BcForceInline BcF32			z() const { return Z_; }

	BcForceInline void			x( BcF32 X ) { X_ = X; }
	BcForceInline void			y( BcF32 Y ) { Y_ = Y; }
	BcForceInline void			z( BcF32 Z ) { Z_ = Z; }

	// Basic Arithmetic
	MaCPUVec3d		operator + ( const MaCPUVec3d& Rhs ) const;
	MaCPUVec3d		operator - ( const MaCPUVec3d& Rhs ) const;
	MaCPUVec3d		operator * ( BcF32 Rhs ) const;
	MaCPUVec3d		operator / ( BcF32 Rhs ) const;
	MaCPUVec3d		operator * ( const MaCPUVec3d& Rhs ) const;
	MaCPUVec3d		operator / ( const MaCPUVec3d& Rhs ) const;

	MaCPUVec3d&		operator += ( const MaCPUVec3d& Rhs );
	MaCPUVec3d&		operator -= ( const MaCPUVec3d& Rhs );
	MaCPUVec3d&		operator *= ( BcF32 Rhs );
	MaCPUVec3d&		operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	MaCPUVec3d		operator - () const;
	BcF32			magnitude() const;
	BcF32			magnitudeSquared() const;      
	MaCPUVec3d		normal() const;
	void			normalise();
	BcF32			dot( const MaCPUVec3d& Rhs ) const;
	MaCPUVec3d		cross( const MaCPUVec3d& Rhs ) const;
	MaCPUVec3d		reflect( const MaCPUVec3d& Normal ) const;

	// Interpolation
	void lerp( const MaCPUVec3d& A, const MaCPUVec3d& B, BcF32 T );

	// Comparison with epsilons
	BcBool			operator == (const MaCPUVec3d& Rhs) const;	
	BcBool			operator != (const MaCPUVec3d& Rhs) const;		

	DECLARE_SWIZZLE2(MaVec2d, x, x);
	DECLARE_SWIZZLE2(MaVec2d, x, y);
	DECLARE_SWIZZLE2(MaVec2d, x, z);
	DECLARE_SWIZZLE2(MaVec2d, y, x);
	DECLARE_SWIZZLE2(MaVec2d, y, y);
	DECLARE_SWIZZLE2(MaVec2d, y, z);
	DECLARE_SWIZZLE2(MaVec2d, z, x);
	DECLARE_SWIZZLE2(MaVec2d, z, y);
	DECLARE_SWIZZLE2(MaVec2d, z, z);

};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline MaCPUVec3d::MaCPUVec3d():
	MaCPUVecQuad( 0.0f, 0.0f, 0.0f )
{

}

BcForceInline MaCPUVec3d::MaCPUVec3d( BcF32 X, BcF32 Y, BcF32 Z ):
	MaCPUVecQuad( X, Y, Z )
{

}

BcForceInline void MaCPUVec3d::set( BcF32 X, BcF32 Y, BcF32 Z )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
}

BcForceInline MaCPUVec3d MaCPUVec3d::operator + ( const MaCPUVec3d& Rhs ) const
{
	return MaCPUVec3d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_ );				
}

BcForceInline MaCPUVec3d MaCPUVec3d::operator - ( const MaCPUVec3d& Rhs ) const
{ 
	return MaCPUVec3d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_ );				
}

BcForceInline MaCPUVec3d MaCPUVec3d::operator * ( BcF32 Rhs ) const
{
	return MaCPUVec3d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs );
}

BcForceInline MaCPUVec3d MaCPUVec3d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return MaCPUVec3d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs);					
}

BcForceInline MaCPUVec3d MaCPUVec3d::operator * ( const MaCPUVec3d& Rhs ) const
{
	return MaCPUVec3d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_ );
}

BcForceInline MaCPUVec3d MaCPUVec3d::operator / ( const MaCPUVec3d& Rhs ) const
{ 
	return MaCPUVec3d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_);					
}

BcForceInline MaCPUVec3d& MaCPUVec3d::operator += ( const MaCPUVec3d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	return (*this);			
}

BcForceInline MaCPUVec3d& MaCPUVec3d::operator -= ( const MaCPUVec3d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	return (*this);			
}

BcForceInline MaCPUVec3d& MaCPUVec3d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	return (*this);				
}

BcForceInline MaCPUVec3d& MaCPUVec3d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	return (*this);				
}

BcForceInline MaCPUVec3d MaCPUVec3d::operator - () const
{
	return MaCPUVec3d( -X_, -Y_, -Z_ );
}

BcForceInline BcF32 MaCPUVec3d::dot( const MaCPUVec3d& Rhs ) const
{
	return ( X_ * Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_ * Rhs.Z_ );
}

BcForceInline BcBool MaCPUVec3d::operator != ( const MaCPUVec3d &Rhs ) const
{
	return !((*this)==Rhs);
}

BcBool BcCheckFloat( MaCPUVec3d T );

#endif // __BCCPUVEC3D_H__

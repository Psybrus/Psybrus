/**************************************************************************
*
* File:		BcCPUVec3d.h
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

#include "Base/BcCPUVecQuad.h"

//////////////////////////////////////////////////////////////////////////
// BcCPUVec3d
class BcCPUVec3d: public BcCPUVecQuad
{
public:
	// Ctors
	BcCPUVec3d();
	BcCPUVec3d( BcF32 X, BcF32 Y, BcF32 Z );
	BcCPUVec3d( const class BcCPUVec2d& Rhs, BcF32 Z );
	BcCPUVec3d( const BcChar* pString );

	void			set( BcF32 X, BcF32 Y, BcF32 Z );

	// Accessors
	BcForceInline BcF32			x() const { return X_; }
	BcForceInline BcF32			y() const { return Y_; }
	BcForceInline BcF32			z() const { return Z_; }

	BcForceInline void			x( BcF32 X ) { X_ = X; }
	BcForceInline void			y( BcF32 Y ) { Y_ = Y; }
	BcForceInline void			z( BcF32 Z ) { Z_ = Z; }

	// Basic Arithmetic
	BcCPUVec3d		operator + ( const BcCPUVec3d& Rhs ) const;
	BcCPUVec3d		operator - ( const BcCPUVec3d& Rhs ) const;
	BcCPUVec3d		operator * ( BcF32 Rhs ) const;
	BcCPUVec3d		operator / ( BcF32 Rhs ) const;
	BcCPUVec3d		operator * ( const BcCPUVec3d& Rhs ) const;
	BcCPUVec3d		operator / ( const BcCPUVec3d& Rhs ) const;

	BcCPUVec3d&		operator += ( const BcCPUVec3d& Rhs );
	BcCPUVec3d&		operator -= ( const BcCPUVec3d& Rhs );
	BcCPUVec3d&		operator *= ( BcF32 Rhs );
	BcCPUVec3d&		operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	BcCPUVec3d		operator - () const;
	BcF32			magnitude() const;
	BcF32			magnitudeSquared() const;      
	BcCPUVec3d		normal() const;
	void			normalise();
	BcF32			dot( const BcCPUVec3d& Rhs ) const;
	BcCPUVec3d		cross( const BcCPUVec3d& Rhs ) const;
	BcCPUVec3d		reflect( const BcCPUVec3d& Normal ) const;

	// Interpolation
	void lerp( const BcCPUVec3d& A, const BcCPUVec3d& B, BcF32 T );

	// Comparison with epsilons
	BcBool			operator == (const BcCPUVec3d& Rhs) const;	
	BcBool			operator != (const BcCPUVec3d& Rhs) const;		
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcCPUVec3d::BcCPUVec3d():
	BcCPUVecQuad( 0.0f, 0.0f, 0.0f )
{

}

BcForceInline BcCPUVec3d::BcCPUVec3d( BcF32 X, BcF32 Y, BcF32 Z ):
	BcCPUVecQuad( X, Y, Z )
{

}

BcForceInline void BcCPUVec3d::set( BcF32 X, BcF32 Y, BcF32 Z )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
}

BcForceInline BcCPUVec3d BcCPUVec3d::operator + ( const BcCPUVec3d& Rhs ) const
{
	return BcCPUVec3d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_ );				
}

BcForceInline BcCPUVec3d BcCPUVec3d::operator - ( const BcCPUVec3d& Rhs ) const
{ 
	return BcCPUVec3d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_ );				
}

BcForceInline BcCPUVec3d BcCPUVec3d::operator * ( BcF32 Rhs ) const
{
	return BcCPUVec3d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs );
}

BcForceInline BcCPUVec3d BcCPUVec3d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return BcCPUVec3d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs);					
}

BcForceInline BcCPUVec3d BcCPUVec3d::operator * ( const BcCPUVec3d& Rhs ) const
{
	return BcCPUVec3d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_ );
}

BcForceInline BcCPUVec3d BcCPUVec3d::operator / ( const BcCPUVec3d& Rhs ) const
{ 
	return BcCPUVec3d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_);					
}

BcForceInline BcCPUVec3d& BcCPUVec3d::operator += ( const BcCPUVec3d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	return (*this);			
}

BcForceInline BcCPUVec3d& BcCPUVec3d::operator -= ( const BcCPUVec3d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	return (*this);			
}

BcForceInline BcCPUVec3d& BcCPUVec3d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	return (*this);				
}

BcForceInline BcCPUVec3d& BcCPUVec3d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	return (*this);				
}

BcForceInline BcCPUVec3d BcCPUVec3d::operator - () const
{
	return BcCPUVec3d( -X_, -Y_, -Z_ );
}

BcForceInline BcF32 BcCPUVec3d::dot( const BcCPUVec3d& Rhs ) const
{
	return ( X_ * Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_ * Rhs.Z_ );
}

BcForceInline BcBool BcCPUVec3d::operator != ( const BcCPUVec3d &Rhs ) const
{
	return !((*this)==Rhs);
}

BcBool BcCheckFloat( BcCPUVec3d T );

#endif // __BCCPUVEC3D_H__

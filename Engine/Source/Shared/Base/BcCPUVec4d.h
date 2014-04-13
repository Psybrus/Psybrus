/**************************************************************************
*
* File:		BcCPUVec4d.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __BCCPUVEC4D_H__
#define __BCCPUVEC4D_H__

#include "Base/BcCPUVecQuad.h"

//////////////////////////////////////////////////////////////////////////
// BcCPUVec4d
class BcCPUVec4d: public BcCPUVecQuad
{
public:
	// Ctors
	BcCPUVec4d(){}
	BcCPUVec4d( const class BcCPUVec2d& Rhs );
	BcCPUVec4d( const class BcCPUVec3d& Rhs, BcF32 W = 0.0f );
	BcCPUVec4d( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W );
	BcCPUVec4d( const BcChar* pString );

	void			set( BcF32 lX, BcF32 lY, BcF32 lZ, BcF32 lW );

	// Accessors
	BcForceInline BcF32			x() const { return X_; }
	BcForceInline BcF32			y() const { return Y_; }
	BcForceInline BcF32			z() const { return Z_; }
	BcForceInline BcF32			w() const { return W_; }

	BcForceInline void			x( BcF32 X ) { X_ = X; }
	BcForceInline void			y( BcF32 Y ) { Y_ = Y; }
	BcForceInline void			z( BcF32 Z ) { Z_ = Z; }
	BcForceInline void			w( BcF32 W ) { W_ = W; }

	// Basic Arithmetic
	BcCPUVec4d			operator + ( const BcCPUVec4d& Rhs ) const;
	BcCPUVec4d			operator - ( const BcCPUVec4d& Rhs ) const;
	BcCPUVec4d			operator * ( BcF32 Rhs ) const;
	BcCPUVec4d			operator / ( BcF32 Rhs ) const;
	BcCPUVec4d			operator * ( const BcCPUVec4d& Rhs ) const;
	BcCPUVec4d			operator / ( const BcCPUVec4d& Rhs ) const;
	BcCPUVec4d&			operator += ( const BcCPUVec4d& Rhs );
	BcCPUVec4d&			operator -= ( const BcCPUVec4d& Rhs );
	BcCPUVec4d&			operator *= ( BcF32 Rhs );
	BcCPUVec4d&			operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	BcCPUVec4d			operator - () const;
	BcF32				magnitude() const;
	BcF32				magnitudeSquared() const;
	BcF32				dot( const BcCPUVec4d& Rhs ) const;
	void				normalise();
	void				normalise3();
	BcCPUVec4d			normal() const;
	class BcCPUVec3d	normal3() const;

	// Interpolation
	void lerp(const BcCPUVec4d& a, const BcCPUVec4d& b, BcF32 t);

	// Comparison with epsilons
	BcBool			operator == (const BcCPUVec4d& Rhs) const;	
	BcBool			operator != (const BcCPUVec4d& Rhs) const;		      
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcCPUVec4d::BcCPUVec4d( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W ):
	BcCPUVecQuad( X, Y, Z, W )
{

}

BcForceInline void BcCPUVec4d::set( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W  )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
	W_ = W;
}

BcForceInline BcCPUVec4d BcCPUVec4d::operator + ( const BcCPUVec4d& Rhs ) const
{
	return BcCPUVec4d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_, W_ + Rhs.W_ );
}

BcForceInline BcCPUVec4d BcCPUVec4d::operator - ( const BcCPUVec4d& Rhs ) const
{ 
	return BcCPUVec4d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_, W_ - Rhs.W_ );				
}

BcForceInline BcCPUVec4d BcCPUVec4d::operator * ( BcF32 Rhs ) const
{
	return BcCPUVec4d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs, W_ * Rhs );
}


BcForceInline BcCPUVec4d BcCPUVec4d::operator / ( const BcCPUVec4d& Rhs ) const
{ 
	return BcCPUVec4d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_, W_ / Rhs.W_ );					
}

BcForceInline BcCPUVec4d BcCPUVec4d::operator * ( const BcCPUVec4d& Rhs ) const
{
	return BcCPUVec4d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_, W_ * Rhs.W_ );
}


BcForceInline BcCPUVec4d BcCPUVec4d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return BcCPUVec4d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs, W_ * InvRhs );					
}

BcForceInline BcCPUVec4d& BcCPUVec4d::operator += ( const BcCPUVec4d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	W_ += Rhs.W_;
	return (*this);			
}

BcForceInline BcCPUVec4d& BcCPUVec4d::operator -= (const BcCPUVec4d& Rhs) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	W_ -= Rhs.W_;
	return (*this);			
}

BcForceInline BcCPUVec4d& BcCPUVec4d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	W_ *= Rhs;
	return (*this);				
}

BcForceInline BcCPUVec4d& BcCPUVec4d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	W_ *= InvRhs;
	return (*this);				
}

BcForceInline BcCPUVec4d BcCPUVec4d::operator - () const
{
	return BcCPUVec4d( -X_, -Y_, -Z_, -W_ );
}

BcForceInline BcF32 BcCPUVec4d::magnitude() const
{
	return BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcForceInline BcF32 BcCPUVec4d::magnitudeSquared() const
{
	return ( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcForceInline void BcCPUVec4d::normalise()
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return;
	}

	const BcF32 InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
	Z_ *= InvMag;
	W_ *= InvMag;
}

BcForceInline void BcCPUVec4d::normalise3()
{
	BcF32 Mag = BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ );

	if ( Mag == 0.0f )
	{
		return;
	}

	const BcF32 InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
	Z_ *= InvMag;
}

BcForceInline BcCPUVec4d BcCPUVec4d::normal() const
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return BcCPUVec4d(0,0,0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return BcCPUVec4d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag, W_ * InvMag );
}


BcForceInline BcF32 BcCPUVec4d::dot( const BcCPUVec4d& Rhs ) const
{
	return ( X_* Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_* Rhs.Z_ )+( W_ * Rhs.W_ );
}

BcForceInline BcBool BcCPUVec4d::operator == ( const BcCPUVec4d &Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
			 ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) &&
			 ( BcAbs( W_ - Rhs.W_ ) < BcVecEpsilon ) );
}

BcForceInline BcBool BcCPUVec4d::operator != ( const BcCPUVec4d &Rhs ) const
{
	return !( (*this) == Rhs );
}

inline BcBool BcCheckFloat( BcCPUVec4d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() ) && BcCheckFloat( T.z() ) && BcCheckFloat( T.w() );
}

#endif // __BCCPUVEC4D_H__

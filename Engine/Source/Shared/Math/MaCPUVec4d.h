/**************************************************************************
*
* File:		MaCPUVec4d.h
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

#include "Math/MaCPUVecQuad.h"

//////////////////////////////////////////////////////////////////////////
// MaCPUVec4d
class MaCPUVec4d: public MaCPUVecQuad
{
public:
	// Ctors
	MaCPUVec4d(){}
	MaCPUVec4d( const class MaCPUVec2d& Rhs );
	MaCPUVec4d( const class MaCPUVec3d& Rhs, BcF32 W = 0.0f );
	MaCPUVec4d( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W );
	MaCPUVec4d( const BcChar* pString );

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
	MaCPUVec4d			operator + ( const MaCPUVec4d& Rhs ) const;
	MaCPUVec4d			operator - ( const MaCPUVec4d& Rhs ) const;
	MaCPUVec4d			operator * ( BcF32 Rhs ) const;
	MaCPUVec4d			operator / ( BcF32 Rhs ) const;
	MaCPUVec4d			operator * ( const MaCPUVec4d& Rhs ) const;
	MaCPUVec4d			operator / ( const MaCPUVec4d& Rhs ) const;
	MaCPUVec4d&			operator += ( const MaCPUVec4d& Rhs );
	MaCPUVec4d&			operator -= ( const MaCPUVec4d& Rhs );
	MaCPUVec4d&			operator *= ( BcF32 Rhs );
	MaCPUVec4d&			operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	MaCPUVec4d			operator - () const;
	BcF32				magnitude() const;
	BcF32				magnitudeSquared() const;
	BcF32				dot( const MaCPUVec4d& Rhs ) const;
	void				normalise();
	void				normalise3();
	MaCPUVec4d			normal() const;
	class MaCPUVec3d	normal3() const;

	// Interpolation
	void lerp(const MaCPUVec4d& a, const MaCPUVec4d& b, BcF32 t);

	// Comparison with epsilons
	BcBool			operator == (const MaCPUVec4d& Rhs) const;	
	BcBool			operator != (const MaCPUVec4d& Rhs) const;		      
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline MaCPUVec4d::MaCPUVec4d( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W ):
	MaCPUVecQuad( X, Y, Z, W )
{

}

BcForceInline void MaCPUVec4d::set( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W  )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
	W_ = W;
}

BcForceInline MaCPUVec4d MaCPUVec4d::operator + ( const MaCPUVec4d& Rhs ) const
{
	return MaCPUVec4d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_, W_ + Rhs.W_ );
}

BcForceInline MaCPUVec4d MaCPUVec4d::operator - ( const MaCPUVec4d& Rhs ) const
{ 
	return MaCPUVec4d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_, W_ - Rhs.W_ );				
}

BcForceInline MaCPUVec4d MaCPUVec4d::operator * ( BcF32 Rhs ) const
{
	return MaCPUVec4d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs, W_ * Rhs );
}


BcForceInline MaCPUVec4d MaCPUVec4d::operator / ( const MaCPUVec4d& Rhs ) const
{ 
	return MaCPUVec4d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_, W_ / Rhs.W_ );					
}

BcForceInline MaCPUVec4d MaCPUVec4d::operator * ( const MaCPUVec4d& Rhs ) const
{
	return MaCPUVec4d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_, W_ * Rhs.W_ );
}


BcForceInline MaCPUVec4d MaCPUVec4d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return MaCPUVec4d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs, W_ * InvRhs );					
}

BcForceInline MaCPUVec4d& MaCPUVec4d::operator += ( const MaCPUVec4d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	W_ += Rhs.W_;
	return (*this);			
}

BcForceInline MaCPUVec4d& MaCPUVec4d::operator -= (const MaCPUVec4d& Rhs) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	W_ -= Rhs.W_;
	return (*this);			
}

BcForceInline MaCPUVec4d& MaCPUVec4d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	W_ *= Rhs;
	return (*this);				
}

BcForceInline MaCPUVec4d& MaCPUVec4d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	W_ *= InvRhs;
	return (*this);				
}

BcForceInline MaCPUVec4d MaCPUVec4d::operator - () const
{
	return MaCPUVec4d( -X_, -Y_, -Z_, -W_ );
}


BcForceInline BcF32 MaCPUVec4d::dot( const MaCPUVec4d& Rhs ) const
{
	return ( X_* Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_* Rhs.Z_ )+( W_ * Rhs.W_ );
}

BcForceInline BcBool MaCPUVec4d::operator != ( const MaCPUVec4d &Rhs ) const
{
	return !( (*this) == Rhs );
}

BcBool BcCheckFloat( MaCPUVec4d T );

#endif // __BCCPUVEC4D_H__

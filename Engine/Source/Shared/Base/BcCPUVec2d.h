/**************************************************************************
*
* File:		BcCPUVec2d.h
* Author: 	Neil Richardson
* Ver/Date:
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __BCCPUVEC2D_H__
#define __BCCPUVEC2D_H__

#include "Base/BcCPUVecQuad.h"

//////////////////////////////////////////////////////////////////////////
// BcCPUVec2d
class BcCPUVec2d: public BcCPUVecQuad
{
public:
	// Ctors
	BcCPUVec2d();
	BcCPUVec2d( BcF32 lX, BcF32 lY );
	BcCPUVec2d( const BcChar* pString );

	void			set( BcF32 X, BcF32 Y );

	// Accessors
	BcForceInline BcF32			x() const { return X_; }
	BcForceInline BcF32			y() const { return Y_; }

	BcForceInline void			x( BcF32 X ) { X_ = X; }
	BcForceInline void			y( BcF32 Y ) { Y_ = Y; }

	// Basic Arithmetic
	BcCPUVec2d		operator + ( const BcCPUVec2d& Rhs ) const;
	BcCPUVec2d		operator - ( const BcCPUVec2d& Rhs ) const;
	BcCPUVec2d		operator * ( const BcCPUVec2d& Rhs ) const;
	BcCPUVec2d		operator / ( const BcCPUVec2d& Rhs ) const;
	BcCPUVec2d		operator * ( BcF32 Rhs ) const;
	BcCPUVec2d		operator / ( BcF32 Rhs ) const;

	BcCPUVec2d&		operator += ( const BcCPUVec2d& Rhs );
	BcCPUVec2d&		operator -= ( const BcCPUVec2d& Rhs );
	BcCPUVec2d&		operator *= ( BcF32 Rhs );
	BcCPUVec2d&		operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	BcCPUVec2d		operator - () const;
	BcF32			magnitude() const;
	BcF32			magnitudeSquared() const;
	BcF32			dot( const BcCPUVec2d& Rhs ) const;
	BcCPUVec2d		cross() const;

	BcCPUVec2d		normal() const;
	void			normalise();

	// Interpolation
	void			lerp( const BcCPUVec2d& A, const BcCPUVec2d& B, BcF32 T );	

	// Comparison with epsilons
	BcBool			operator == ( const BcCPUVec2d& Rhs ) const;	
	BcBool			operator != ( const BcCPUVec2d& Rhs ) const;		      
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcCPUVec2d::BcCPUVec2d()
{

}

BcForceInline BcCPUVec2d::BcCPUVec2d( BcF32 X, BcF32 Y ):
	BcCPUVecQuad( X, Y )
{

}

BcForceInline void BcCPUVec2d::set( BcF32 X, BcF32 Y )
{
	X_ = X;
	Y_ = Y;
}

BcForceInline BcCPUVec2d BcCPUVec2d::operator + ( const BcCPUVec2d& Rhs ) const
{
	return BcCPUVec2d( X_ + Rhs.X_, Y_ + Rhs.Y_ );				
}

BcForceInline BcCPUVec2d BcCPUVec2d::operator - ( const BcCPUVec2d& Rhs ) const 
{ 
	return BcCPUVec2d( X_ - Rhs.X_, Y_ - Rhs.Y_ );				
}

BcForceInline BcCPUVec2d BcCPUVec2d::operator * ( const BcCPUVec2d& Rhs )  const	
{
	return BcCPUVec2d( X_ * Rhs.X_, Y_ * Rhs.Y_ );
}

BcForceInline BcCPUVec2d BcCPUVec2d::operator / ( const BcCPUVec2d& Rhs ) const
{ 
	return BcCPUVec2d( X_ / Rhs.X_, Y_ / Rhs.Y_ );	
}

BcForceInline BcCPUVec2d BcCPUVec2d::operator * ( BcF32 Rhs )  const	
{
	return BcCPUVec2d( X_ * Rhs, Y_ * Rhs );
}

BcForceInline BcCPUVec2d BcCPUVec2d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return BcCPUVec2d( X_ * InvRhs, Y_ * InvRhs );					
}

BcForceInline BcCPUVec2d& BcCPUVec2d::operator += ( const BcCPUVec2d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	return (*this);			
}

BcForceInline BcCPUVec2d& BcCPUVec2d::operator -= ( const BcCPUVec2d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	return (*this);			
}

BcForceInline BcCPUVec2d& BcCPUVec2d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	return (*this);				
}

BcForceInline BcCPUVec2d& BcCPUVec2d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	return (*this);
}

BcForceInline BcCPUVec2d BcCPUVec2d::operator - () const
{
	return BcCPUVec2d( -X_, -Y_ );
}

BcForceInline BcF32 BcCPUVec2d::dot( const BcCPUVec2d& Rhs ) const
{
	return ( X_ * Rhs.X_ ) + ( Y_ * Rhs.Y_ );
}

BcForceInline BcCPUVec2d BcCPUVec2d::cross() const
{
	return BcCPUVec2d( -Y_, X_ );
}

BcForceInline BcBool BcCPUVec2d::operator != ( const BcCPUVec2d& Rhs ) const
{
	return !( (*this) == Rhs );
}

BcBool BcCheckFloat( BcCPUVec2d T );

#endif // __BCCPUVEC2D_H__

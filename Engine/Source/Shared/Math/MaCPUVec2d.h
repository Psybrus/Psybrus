/**************************************************************************
*
* File:		MaCPUVec2d.h
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

#include "Base/BcTypes.h"
#include "Reflection/ReUtility.h"

//////////////////////////////////////////////////////////////////////////
// MaCPUVec2d
class MaCPUVec2d
{
public:
	REFLECTION_DECLARE_BASIC( MaCPUVec2d );

public:
	// Ctors
	MaCPUVec2d();
	MaCPUVec2d( BcF32 lX, BcF32 lY );
	MaCPUVec2d( const BcChar* pString );

	void			set( BcF32 X, BcF32 Y );

	// Accessors
	BcForceInline BcF32			x() const { return X_; }
	BcForceInline BcF32			y() const { return Y_; }

	BcForceInline void			x( BcF32 X ) { X_ = X; }
	BcForceInline void			y( BcF32 Y ) { Y_ = Y; }

	// Basic Arithmetic
	MaCPUVec2d		operator + ( const MaCPUVec2d& Rhs ) const;
	MaCPUVec2d		operator - ( const MaCPUVec2d& Rhs ) const;
	MaCPUVec2d		operator * ( const MaCPUVec2d& Rhs ) const;
	MaCPUVec2d		operator / ( const MaCPUVec2d& Rhs ) const;
	MaCPUVec2d		operator * ( BcF32 Rhs ) const;
	MaCPUVec2d		operator / ( BcF32 Rhs ) const;

	MaCPUVec2d&		operator += ( const MaCPUVec2d& Rhs );
	MaCPUVec2d&		operator -= ( const MaCPUVec2d& Rhs );
	MaCPUVec2d&		operator *= ( BcF32 Rhs );
	MaCPUVec2d&		operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	MaCPUVec2d		operator - () const;
	BcF32			magnitude() const;
	BcF32			magnitudeSquared() const;
	BcF32			dot( const MaCPUVec2d& Rhs ) const;
	MaCPUVec2d		cross() const;

	MaCPUVec2d		normal() const;
	void			normalise();

	// Interpolation
	void			lerp( const MaCPUVec2d& A, const MaCPUVec2d& B, BcF32 T );	

	// Comparison with epsilons
	BcBool			operator == ( const MaCPUVec2d& Rhs ) const;	
	BcBool			operator != ( const MaCPUVec2d& Rhs ) const;		      

protected:
	BcF32 X_, Y_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline MaCPUVec2d::MaCPUVec2d():
	X_( 0.0f ), // todo, remove these.
	Y_( 0.0f ) // todo, remove these.
{

}

BcForceInline MaCPUVec2d::MaCPUVec2d( BcF32 X, BcF32 Y ):
	X_( X ),
	Y_( Y )
{

}

BcForceInline void MaCPUVec2d::set( BcF32 X, BcF32 Y )
{
	X_ = X;
	Y_ = Y;
}

BcForceInline MaCPUVec2d MaCPUVec2d::operator + ( const MaCPUVec2d& Rhs ) const
{
	return MaCPUVec2d( X_ + Rhs.X_, Y_ + Rhs.Y_ );				
}

BcForceInline MaCPUVec2d MaCPUVec2d::operator - ( const MaCPUVec2d& Rhs ) const 
{ 
	return MaCPUVec2d( X_ - Rhs.X_, Y_ - Rhs.Y_ );				
}

BcForceInline MaCPUVec2d MaCPUVec2d::operator * ( const MaCPUVec2d& Rhs )  const	
{
	return MaCPUVec2d( X_ * Rhs.X_, Y_ * Rhs.Y_ );
}

BcForceInline MaCPUVec2d MaCPUVec2d::operator / ( const MaCPUVec2d& Rhs ) const
{ 
	return MaCPUVec2d( X_ / Rhs.X_, Y_ / Rhs.Y_ );	
}

BcForceInline MaCPUVec2d MaCPUVec2d::operator * ( BcF32 Rhs )  const	
{
	return MaCPUVec2d( X_ * Rhs, Y_ * Rhs );
}

BcForceInline MaCPUVec2d MaCPUVec2d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return MaCPUVec2d( X_ * InvRhs, Y_ * InvRhs );					
}

BcForceInline MaCPUVec2d& MaCPUVec2d::operator += ( const MaCPUVec2d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	return (*this);			
}

BcForceInline MaCPUVec2d& MaCPUVec2d::operator -= ( const MaCPUVec2d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	return (*this);			
}

BcForceInline MaCPUVec2d& MaCPUVec2d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	return (*this);				
}

BcForceInline MaCPUVec2d& MaCPUVec2d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	return (*this);
}

BcForceInline MaCPUVec2d MaCPUVec2d::operator - () const
{
	return MaCPUVec2d( -X_, -Y_ );
}

BcForceInline BcF32 MaCPUVec2d::dot( const MaCPUVec2d& Rhs ) const
{
	return ( X_ * Rhs.X_ ) + ( Y_ * Rhs.Y_ );
}

BcForceInline MaCPUVec2d MaCPUVec2d::cross() const
{
	return MaCPUVec2d( -Y_, X_ );
}

BcForceInline BcBool MaCPUVec2d::operator != ( const MaCPUVec2d& Rhs ) const
{
	return !( (*this) == Rhs );
}

BcBool BcCheckFloat( MaCPUVec2d T );

#endif // __BCCPUVEC2D_H__

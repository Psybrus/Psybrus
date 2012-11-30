/**************************************************************************
*
* File:		BcFixedVectors.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*
*
*
* 
**************************************************************************/

#ifndef	__BCFIXEDVECTORS_H__
#define __BCFIXEDVECTORS_H__

#include "Base/BcMath.h"
#include "Base/BcTypes.h"
#include "Base/BcFixed.h"

//////////////////////////////////////////////////////////////////////////
// BcFixedVecQuad
struct BcFixedVecQuad
{
public:
	BcForceInline BcFixedVecQuad():
		X_( 0.0f ),
		Y_( 0.0f ),
		Z_( 0.0f ),
		W_( 0.0f )
	{}

	BcForceInline BcFixedVecQuad( BcFixed<> X, BcFixed<> Y ):
		X_( X ),
		Y_( Y )
	{}

	BcForceInline BcFixedVecQuad( BcFixed<> X, BcFixed<> Y, BcFixed<> Z ):
		X_( X ),
		Y_( Y ),
		Z_( Z )
	{}

	BcForceInline BcFixedVecQuad( BcFixed<> X, BcFixed<> Y, BcFixed<> Z, BcFixed<> W ):
		X_( X ),
		Y_( Y ),
		Z_( Z ),
		W_( W )
	{}

protected:
	BcAlign( BcFixed<> X_, 16 );
	BcFixed<> Y_;
	BcFixed<> Z_;
	BcFixed<> W_;
};

//////////////////////////////////////////////////////////////////////////
// BcFixedVec2d
class BcFixedVec2d: public BcFixedVecQuad
{
public:
	// Ctors
	BcFixedVec2d();
	BcFixedVec2d( BcFixed<> lX, BcFixed<> lY );

	void			set( BcFixed<> X, BcFixed<> Y );

	// Accessors
	BcForceInline BcFixed<>			x() const { return X_; }
	BcForceInline BcFixed<>			y() const { return Y_; }

	BcForceInline void			x( BcFixed<> X ) { X_ = X; }
	BcForceInline void			y( BcFixed<> Y ) { Y_ = Y; }

	// Basic Arithmetic
	BcFixedVec2d			operator + ( const BcFixedVec2d& Rhs ) const;
	BcFixedVec2d			operator - ( const BcFixedVec2d& Rhs ) const;
	BcFixedVec2d			operator * ( const BcFixedVec2d& Rhs ) const;
	BcFixedVec2d			operator / ( const BcFixedVec2d& Rhs ) const;
	BcFixedVec2d			operator * ( BcFixed<> Rhs ) const;
	BcFixedVec2d			operator / ( BcFixed<> Rhs ) const;

	BcFixedVec2d&		operator += ( const BcFixedVec2d& Rhs );
	BcFixedVec2d&		operator -= ( const BcFixedVec2d& Rhs );
	BcFixedVec2d&		operator *= ( BcFixed<> Rhs );
	BcFixedVec2d&		operator /= ( BcFixed<> Rhs );

	// Slightly more advanced arithmetic
	BcFixedVec2d			operator - () const;
	BcFixed<>			magnitude() const;
	BcFixed<>			magnitudeSquared() const;
	BcFixed<>			dot( const BcFixedVec2d& Rhs ) const;

	BcFixedVec2d			normal() const;
	void			normalise();

	// Interpolation
	void			lerp( const BcFixedVec2d& A, const BcFixedVec2d& B, BcFixed<> T );	

	// Comparison with epsilons
	BcBool			operator == ( const BcFixedVec2d& Rhs ) const;	
	BcBool			operator != ( const BcFixedVec2d& Rhs ) const;		      
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcFixedVec2d::BcFixedVec2d()
{

}

BcForceInline BcFixedVec2d::BcFixedVec2d( BcFixed<> X, BcFixed<> Y ):
	BcFixedVecQuad( X, Y )
{

}

BcForceInline void BcFixedVec2d::set( BcFixed<> X, BcFixed<> Y )
{
	X_ = X;
	Y_ = Y;
}

BcForceInline BcFixedVec2d BcFixedVec2d::operator + ( const BcFixedVec2d& Rhs ) const
{
	return BcFixedVec2d( X_ + Rhs.X_, Y_ + Rhs.Y_ );				
}

BcForceInline BcFixedVec2d BcFixedVec2d::operator - ( const BcFixedVec2d& Rhs ) const 
{ 
	return BcFixedVec2d( X_ - Rhs.X_, Y_ - Rhs.Y_ );				
}

BcForceInline BcFixedVec2d BcFixedVec2d::operator * ( const BcFixedVec2d& Rhs )  const	
{
	return BcFixedVec2d( X_ * Rhs.X_, Y_ * Rhs.Y_ );
}

BcForceInline BcFixedVec2d BcFixedVec2d::operator / ( const BcFixedVec2d& Rhs ) const
{ 
	return BcFixedVec2d( X_ / Rhs.X_, Y_ / Rhs.Y_ );	
}

BcForceInline BcFixedVec2d BcFixedVec2d::operator * ( BcFixed<> Rhs )  const	
{
	return BcFixedVec2d( X_ * Rhs, Y_ * Rhs );
}

BcForceInline BcFixedVec2d BcFixedVec2d::operator / ( BcFixed<> Rhs ) const
{ 
	const BcFixed<> InvRhs = 1.0f / Rhs;
	return BcFixedVec2d( X_ * InvRhs, Y_ * InvRhs );					
}

BcForceInline BcFixedVec2d& BcFixedVec2d::operator += ( const BcFixedVec2d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	return (*this);			
}

BcForceInline BcFixedVec2d& BcFixedVec2d::operator -= ( const BcFixedVec2d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	return (*this);			
}

BcForceInline BcFixedVec2d& BcFixedVec2d::operator *= ( BcFixed<> Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	return (*this);				
}

BcForceInline BcFixedVec2d& BcFixedVec2d::operator /= ( BcFixed<> Rhs ) 	
{ 
	const BcFixed<> InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	return (*this);
}

BcForceInline BcFixedVec2d BcFixedVec2d::operator - () const
{
	return BcFixedVec2d( -X_, -Y_ );
}

BcForceInline BcFixed<> BcFixedVec2d::magnitudeSquared() const
{
	return ( ( X_ * X_ ) + ( Y_ * Y_ ) );
}

BcForceInline BcFixed<> BcFixedVec2d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

BcForceInline BcFixedVec2d BcFixedVec2d::normal() const
{
	BcFixed<> Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return BcFixedVec2d(0,0);
	}
	
	const BcFixed<> InvMag = 1.0f / Mag;
	return BcFixedVec2d( X_ * InvMag, Y_ * InvMag );
}

BcForceInline void BcFixedVec2d::normalise()
{
	BcFixed<> Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return;
	}
	
	const BcFixed<> InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
}

BcForceInline BcFixed<> BcFixedVec2d::dot( const BcFixedVec2d& Rhs ) const
{
	return ( X_ * Rhs.X_ ) + ( Y_ * Rhs.Y_ );
}

BcForceInline BcBool BcFixedVec2d::operator == ( const BcFixedVec2d& Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) && ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) );
}

BcForceInline BcBool BcFixedVec2d::operator != ( const BcFixedVec2d& Rhs ) const
{
	return !( (*this) == Rhs );
}

//////////////////////////////////////////////////////////////////////////
// BcFixedVec3d
class BcFixedVec3d: public BcFixedVecQuad
{
public:
	// Ctors
	BcFixedVec3d();
	BcFixedVec3d( BcFixed<> X, BcFixed<> Y, BcFixed<> Z );

	void			set( BcFixed<> X, BcFixed<> Y, BcFixed<> Z );

	// Accessors
	BcForceInline BcFixed<>			x() const { return X_; }
	BcForceInline BcFixed<>			y() const { return Y_; }
	BcForceInline BcFixed<>			z() const { return Z_; }

	BcForceInline void			x( BcFixed<> X ) { X_ = X; }
	BcForceInline void			y( BcFixed<> Y ) { Y_ = Y; }
	BcForceInline void			z( BcFixed<> Z ) { Z_ = Z; }

	// Basic Arithmetic
	BcFixedVec3d			operator + ( const BcFixedVec3d& Rhs ) const;
	BcFixedVec3d			operator - ( const BcFixedVec3d& Rhs ) const;
	BcFixedVec3d			operator * ( BcFixed<> Rhs ) const;
	BcFixedVec3d			operator / ( BcFixed<> Rhs ) const;
	BcFixedVec3d			operator * ( const BcFixedVec3d& Rhs ) const;
	BcFixedVec3d			operator / ( const BcFixedVec3d& Rhs ) const;

	BcFixedVec3d&		operator += ( const BcFixedVec3d& Rhs );
	BcFixedVec3d&		operator -= ( const BcFixedVec3d& Rhs );
	BcFixedVec3d&		operator *= ( BcFixed<> Rhs );
	BcFixedVec3d&		operator /= ( BcFixed<> Rhs );

	// Slightly more advanced arithmetic
	BcFixedVec3d			operator - () const;
	BcFixed<>			magnitude() const;
	BcFixed<>			magnitudeSquared() const;      
	BcFixedVec3d			normal() const;
	void			normalise();
	BcFixed<>			dot( const BcFixedVec3d& Rhs ) const;
	BcFixedVec3d			cross( const BcFixedVec3d& Rhs ) const;
	BcFixedVec3d			reflect( const BcFixedVec3d& Normal ) const;

	// Interpolation
	void lerp( const BcFixedVec3d& A, const BcFixedVec3d& B, BcFixed<> T );

	// Comparison with epsilons
	BcBool			operator == (const BcFixedVec3d& Rhs) const;	
	BcBool			operator != (const BcFixedVec3d& Rhs) const;		
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcFixedVec3d::BcFixedVec3d():
	BcFixedVecQuad( 0.0f, 0.0f, 0.0f )
{

}

BcForceInline BcFixedVec3d::BcFixedVec3d( BcFixed<> X, BcFixed<> Y, BcFixed<> Z ):
	BcFixedVecQuad( X, Y, Z )
{

}

BcForceInline void BcFixedVec3d::set( BcFixed<> X, BcFixed<> Y, BcFixed<> Z )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
}

BcForceInline BcFixedVec3d BcFixedVec3d::operator + ( const BcFixedVec3d& Rhs ) const
{
	return BcFixedVec3d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_ );				
}

BcForceInline BcFixedVec3d BcFixedVec3d::operator - ( const BcFixedVec3d& Rhs ) const
{ 
	return BcFixedVec3d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_ );				
}

BcForceInline BcFixedVec3d BcFixedVec3d::operator * ( BcFixed<> Rhs ) const
{
	return BcFixedVec3d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs );
}

BcForceInline BcFixedVec3d BcFixedVec3d::operator / ( BcFixed<> Rhs ) const
{ 
	const BcFixed<> InvRhs = 1.0f / Rhs;
	return BcFixedVec3d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs);					
}

BcForceInline BcFixedVec3d BcFixedVec3d::operator * ( const BcFixedVec3d& Rhs ) const
{
	return BcFixedVec3d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_ );
}

BcForceInline BcFixedVec3d BcFixedVec3d::operator / ( const BcFixedVec3d& Rhs ) const
{ 
	return BcFixedVec3d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_);					
}

BcForceInline BcFixedVec3d& BcFixedVec3d::operator += ( const BcFixedVec3d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	return (*this);			
}

BcForceInline BcFixedVec3d& BcFixedVec3d::operator -= ( const BcFixedVec3d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	return (*this);			
}

BcForceInline BcFixedVec3d& BcFixedVec3d::operator *= ( BcFixed<> Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	return (*this);				
}

BcForceInline BcFixedVec3d& BcFixedVec3d::operator /= ( BcFixed<> Rhs ) 	
{ 
	const BcFixed<> InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	return (*this);				
}

BcForceInline BcFixedVec3d BcFixedVec3d::operator - () const
{
	return BcFixedVec3d( -X_, -Y_, -Z_ );
}

BcForceInline BcFixed<> BcFixedVec3d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

BcForceInline BcFixed<> BcFixedVec3d::magnitudeSquared() const
{
	return ( X_ * X_ ) + ( Y_ * Y_ ) + ( Z_ * Z_ );
}

BcForceInline BcFixedVec3d BcFixedVec3d::normal() const
{
	BcFixed<> Mag = magnitude();

	if ( Mag == 0.0f )
	{
		return BcFixedVec3d(0,0,0);
	}

	const BcFixed<> InvMag = 1.0f / Mag;
	return BcFixedVec3d(X_ * InvMag, Y_ * InvMag, Z_ * InvMag);
}

BcForceInline void BcFixedVec3d::normalise()
{
	BcFixed<> Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return;
	}
	
	const BcFixed<> InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
	Z_ *= InvMag;
}

BcForceInline BcFixed<> BcFixedVec3d::dot( const BcFixedVec3d& Rhs ) const
{
	return ( X_ * Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_ * Rhs.Z_ );
}

BcForceInline BcFixedVec3d BcFixedVec3d::cross( const BcFixedVec3d& Rhs ) const
{
	return BcFixedVec3d( ( Y_ * Rhs.Z_)  - ( Rhs.Y_ * Z_ ), ( Z_* Rhs.X_ ) - ( Rhs.Z_ * X_ ), ( X_ * Rhs.Y_ ) - ( Rhs.X_ * Y_ ) );
}

BcForceInline BcBool BcFixedVec3d::operator == (const BcFixedVec3d &Rhs) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) );
}

BcForceInline BcFixedVec3d BcFixedVec3d::reflect( const BcFixedVec3d& Normal ) const
{
	return ( *this - ( Normal * ( 2.0f * this->dot( Normal ) ) ) );
}

BcForceInline BcBool BcFixedVec3d::operator != ( const BcFixedVec3d &Rhs ) const
{
	return !((*this)==Rhs);
}

//////////////////////////////////////////////////////////////////////////
// BcFixedVec4d
class BcFixedVec4d: public BcFixedVecQuad
{
public:
	// Ctors
	BcFixedVec4d(){}
	BcFixedVec4d( const BcFixedVec2d& Rhs );
	BcFixedVec4d( const BcFixedVec3d& Rhs, BcFixed<> W = 0.0f );
	BcFixedVec4d( BcFixed<> X, BcFixed<> Y, BcFixed<> Z, BcFixed<> W );

	void			set( BcFixed<> lX, BcFixed<> lY, BcFixed<> lZ, BcFixed<> lW );

	// Accessors
	BcForceInline BcFixed<>			x() const { return X_; }
	BcForceInline BcFixed<>			y() const { return Y_; }
	BcForceInline BcFixed<>			z() const { return Z_; }
	BcForceInline BcFixed<>			w() const { return W_; }

	BcForceInline void			x( BcFixed<> X ) { X_ = X; }
	BcForceInline void			y( BcFixed<> Y ) { Y_ = Y; }
	BcForceInline void			z( BcFixed<> Z ) { Z_ = Z; }
	BcForceInline void			w( BcFixed<> W ) { W_ = W; }

	// Basic Arithmetic
	BcFixedVec4d			operator + ( const BcFixedVec4d& Rhs ) const;
	BcFixedVec4d			operator - ( const BcFixedVec4d& Rhs ) const;
	BcFixedVec4d			operator * ( BcFixed<> Rhs ) const;
	BcFixedVec4d			operator / ( BcFixed<> Rhs ) const;
	BcFixedVec4d			operator * ( const BcFixedVec4d& Rhs ) const;
	BcFixedVec4d			operator / ( const BcFixedVec4d& Rhs ) const;
	BcFixedVec4d&		operator += ( const BcFixedVec4d& Rhs );
	BcFixedVec4d&		operator -= ( const BcFixedVec4d& Rhs );
	BcFixedVec4d&		operator *= ( BcFixed<> Rhs );
	BcFixedVec4d&		operator /= ( BcFixed<> Rhs );

	// Slightly more advanced arithmetic
	BcFixedVec4d			operator - () const;
	BcFixed<>			magnitude() const;
	BcFixed<>			magnitudeSquared() const;
	BcFixed<>			dot( const BcFixedVec4d& Rhs ) const;
	void			normalise();
	void			normalise3();
	BcFixedVec4d			normal() const;
	BcFixedVec3d			normal3() const;

	// Interpolation
	void lerp(const BcFixedVec4d& a, const BcFixedVec4d& b, BcFixed<> t);

	// Comparison with epsilons
	BcBool			operator == (const BcFixedVec4d& Rhs) const;	
	BcBool			operator != (const BcFixedVec4d& Rhs) const;		      
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcFixedVec4d::BcFixedVec4d( const BcFixedVec2d& Rhs ):
	BcFixedVecQuad( Rhs.x(), Rhs.y(), 0.0f, 0.0f )
{

}

BcForceInline BcFixedVec4d::BcFixedVec4d( const BcFixedVec3d& Rhs, BcFixed<> W ):
	BcFixedVecQuad( Rhs.x(), Rhs.y(), Rhs.z(), W )
{

}

BcForceInline BcFixedVec4d::BcFixedVec4d( BcFixed<> X, BcFixed<> Y, BcFixed<> Z, BcFixed<> W ):
	BcFixedVecQuad( X, Y, Z, W )
{

}

BcForceInline void BcFixedVec4d::set( BcFixed<> X, BcFixed<> Y, BcFixed<> Z, BcFixed<> W  )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
	W_ = W;
}

BcForceInline BcFixedVec4d BcFixedVec4d::operator + ( const BcFixedVec4d& Rhs ) const
{
	return BcFixedVec4d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_, W_ + Rhs.W_ );
}

BcForceInline BcFixedVec4d BcFixedVec4d::operator - ( const BcFixedVec4d& Rhs ) const
{ 
	return BcFixedVec4d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_, W_ - Rhs.W_ );				
}

BcForceInline BcFixedVec4d BcFixedVec4d::operator * ( BcFixed<> Rhs ) const
{
	return BcFixedVec4d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs, W_ * Rhs );
}


BcForceInline BcFixedVec4d BcFixedVec4d::operator / ( const BcFixedVec4d& Rhs ) const
{ 
	return BcFixedVec4d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_, W_ / Rhs.W_ );					
}

BcForceInline BcFixedVec4d BcFixedVec4d::operator * ( const BcFixedVec4d& Rhs ) const
{
	return BcFixedVec4d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_, W_ * Rhs.W_ );
}


BcForceInline BcFixedVec4d BcFixedVec4d::operator / ( BcFixed<> Rhs ) const
{ 
	const BcFixed<> InvRhs = 1.0f / Rhs;
	return BcFixedVec4d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs, W_ * InvRhs );					
}

BcForceInline BcFixedVec4d& BcFixedVec4d::operator += ( const BcFixedVec4d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	W_ += Rhs.W_;
	return (*this);			
}

BcForceInline BcFixedVec4d& BcFixedVec4d::operator -= (const BcFixedVec4d& Rhs) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	W_ -= Rhs.W_;
	return (*this);			
}

BcForceInline BcFixedVec4d& BcFixedVec4d::operator *= ( BcFixed<> Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	W_ *= Rhs;
	return (*this);				
}

BcForceInline BcFixedVec4d& BcFixedVec4d::operator /= ( BcFixed<> Rhs ) 	
{ 
	const BcFixed<> InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	W_ *= InvRhs;
	return (*this);				
}

BcForceInline BcFixedVec4d BcFixedVec4d::operator - () const
{
	return BcFixedVec4d( -X_, -Y_, -Z_, -W_ );
}

BcForceInline BcFixed<> BcFixedVec4d::magnitude() const
{
	return BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcForceInline BcFixed<> BcFixedVec4d::magnitudeSquared() const
{
	return ( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcForceInline void BcFixedVec4d::normalise()
{
	BcFixed<> Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return;
	}

	const BcFixed<> InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
	Z_ *= InvMag;
	W_ *= InvMag;
}

BcForceInline void BcFixedVec4d::normalise3()
{
	BcFixed<> Mag = BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ );

	if ( Mag == 0.0f )
	{
		return;
	}

	const BcFixed<> InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
	Z_ *= InvMag;
}

BcForceInline BcFixedVec4d BcFixedVec4d::normal() const
{
	BcFixed<> Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return BcFixedVec4d(0,0,0,0);
	}
	
	const BcFixed<> InvMag = 1.0f / Mag;
	return BcFixedVec4d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag, W_ * InvMag );
}

BcForceInline BcFixedVec3d BcFixedVec4d::normal3() const
{
	BcFixed<> Mag = BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ );
	
	if ( Mag == 0.0f )
	{
		return BcFixedVec3d(0,0,0);
	}
	
	const BcFixed<> InvMag = 1.0f / Mag;
	return BcFixedVec3d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag );
}


BcForceInline BcFixed<> BcFixedVec4d::dot( const BcFixedVec4d& Rhs ) const
{
	return ( X_* Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_* Rhs.Z_ )+( W_ * Rhs.W_ );
}

BcForceInline BcBool BcFixedVec4d::operator == ( const BcFixedVec4d &Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
			 ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) &&
			 ( BcAbs( W_ - Rhs.W_ ) < BcVecEpsilon ) );
}

BcForceInline BcBool BcFixedVec4d::operator != ( const BcFixedVec4d &Rhs ) const
{
	return !( (*this) == Rhs );
}

#endif

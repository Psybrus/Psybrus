/**************************************************************************
*
* File:		BcVectors.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*
*
*
* 
**************************************************************************/

#ifndef	__BCVECTORS_H__
#define __BCVECTORS_H__

#include "Base/BcMath.h"
#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcVecQuad
struct BcVecQuad
{
public:
	BcForceInline BcVecQuad()
	{}

	BcForceInline BcVecQuad( BcF32 X, BcF32 Y ):
		X_( X ),
		Y_( Y )
	{}

	BcForceInline BcVecQuad( BcF32 X, BcF32 Y, BcF32 Z ):
		X_( X ),
		Y_( Y ),
		Z_( Z )
	{}

	BcForceInline BcVecQuad( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W ):
		X_( X ),
		Y_( Y ),
		Z_( Z ),
		W_( W )
	{}

protected:
	BcAlign( BcF32 X_, 16 );
	BcF32 Y_;
	BcF32 Z_;
	BcF32 W_;
};

//////////////////////////////////////////////////////////////////////////
// BcVec2d
class BcVec2d: public BcVecQuad
{
public:
	// Ctors
	BcVec2d();
	BcVec2d( BcF32 lX, BcF32 lY );
	BcVec2d( const BcChar* pString );

	void			set( BcF32 X, BcF32 Y );

	// Accessors
	BcForceInline BcF32			x() const { return X_; }
	BcForceInline BcF32			y() const { return Y_; }

	BcForceInline void			x( BcF32 X ) { X_ = X; }
	BcForceInline void			y( BcF32 Y ) { Y_ = Y; }

	// Basic Arithmetic
	BcVec2d			operator + ( const BcVec2d& Rhs ) const;
	BcVec2d			operator - ( const BcVec2d& Rhs ) const;
	BcVec2d			operator * ( const BcVec2d& Rhs ) const;
	BcVec2d			operator / ( const BcVec2d& Rhs ) const;
	BcVec2d			operator * ( BcF32 Rhs ) const;
	BcVec2d			operator / ( BcF32 Rhs ) const;

	BcVec2d&		operator += ( const BcVec2d& Rhs );
	BcVec2d&		operator -= ( const BcVec2d& Rhs );
	BcVec2d&		operator *= ( BcF32 Rhs );
	BcVec2d&		operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	BcVec2d			operator - () const;
	BcF32			magnitude() const;
	BcF32			magnitudeSquared() const;
	BcF32			dot( const BcVec2d& Rhs ) const;
	BcVec2d			cross() const;

	BcVec2d			normal() const;
	void			normalise();

	// Interpolation
	void			lerp( const BcVec2d& A, const BcVec2d& B, BcF32 T );	

	// Comparison with epsilons
	BcBool			operator == ( const BcVec2d& Rhs ) const;	
	BcBool			operator != ( const BcVec2d& Rhs ) const;		      
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcVec2d::BcVec2d()
{

}

BcForceInline BcVec2d::BcVec2d( BcF32 X, BcF32 Y ):
	BcVecQuad( X, Y )
{

}

BcForceInline void BcVec2d::set( BcF32 X, BcF32 Y )
{
	X_ = X;
	Y_ = Y;
}

BcForceInline BcVec2d BcVec2d::operator + ( const BcVec2d& Rhs ) const
{
	return BcVec2d( X_ + Rhs.X_, Y_ + Rhs.Y_ );				
}

BcForceInline BcVec2d BcVec2d::operator - ( const BcVec2d& Rhs ) const 
{ 
	return BcVec2d( X_ - Rhs.X_, Y_ - Rhs.Y_ );				
}

BcForceInline BcVec2d BcVec2d::operator * ( const BcVec2d& Rhs )  const	
{
	return BcVec2d( X_ * Rhs.X_, Y_ * Rhs.Y_ );
}

BcForceInline BcVec2d BcVec2d::operator / ( const BcVec2d& Rhs ) const
{ 
	return BcVec2d( X_ / Rhs.X_, Y_ / Rhs.Y_ );	
}

BcForceInline BcVec2d BcVec2d::operator * ( BcF32 Rhs )  const	
{
	return BcVec2d( X_ * Rhs, Y_ * Rhs );
}

BcForceInline BcVec2d BcVec2d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return BcVec2d( X_ * InvRhs, Y_ * InvRhs );					
}

BcForceInline BcVec2d& BcVec2d::operator += ( const BcVec2d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	return (*this);			
}

BcForceInline BcVec2d& BcVec2d::operator -= ( const BcVec2d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	return (*this);			
}

BcForceInline BcVec2d& BcVec2d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	return (*this);				
}

BcForceInline BcVec2d& BcVec2d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	return (*this);
}

BcForceInline BcVec2d BcVec2d::operator - () const
{
	return BcVec2d( -X_, -Y_ );
}

BcForceInline BcF32 BcVec2d::magnitudeSquared() const
{
	return ( ( X_ * X_ ) + ( Y_ * Y_ ) );
}

BcForceInline BcF32 BcVec2d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

BcForceInline BcVec2d BcVec2d::normal() const
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return BcVec2d(0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return BcVec2d( X_ * InvMag, Y_ * InvMag );
}

BcForceInline void BcVec2d::normalise()
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return;
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	X_ *= InvMag;
	Y_ *= InvMag;
}

BcForceInline BcF32 BcVec2d::dot( const BcVec2d& Rhs ) const
{
	return ( X_ * Rhs.X_ ) + ( Y_ * Rhs.Y_ );
}

BcForceInline BcVec2d BcVec2d::cross() const
{
	return BcVec2d( -Y_, X_ );
}

BcForceInline BcBool BcVec2d::operator == ( const BcVec2d& Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) && ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) );
}

BcForceInline BcBool BcVec2d::operator != ( const BcVec2d& Rhs ) const
{
	return !( (*this) == Rhs );
}

//////////////////////////////////////////////////////////////////////////
// BcVec3d
class BcVec3d: public BcVecQuad
{
public:
	// Ctors
	BcVec3d();
	BcVec3d( BcF32 X, BcF32 Y, BcF32 Z );
	BcVec3d( const BcVec2d& Rhs, BcF32 Z );
	BcVec3d( const BcChar* pString );

	void			set( BcF32 X, BcF32 Y, BcF32 Z );

	// Accessors
	BcForceInline BcF32			x() const { return X_; }
	BcForceInline BcF32			y() const { return Y_; }
	BcForceInline BcF32			z() const { return Z_; }

	BcForceInline void			x( BcF32 X ) { X_ = X; }
	BcForceInline void			y( BcF32 Y ) { Y_ = Y; }
	BcForceInline void			z( BcF32 Z ) { Z_ = Z; }

	// Basic Arithmetic
	BcVec3d			operator + ( const BcVec3d& Rhs ) const;
	BcVec3d			operator - ( const BcVec3d& Rhs ) const;
	BcVec3d			operator * ( BcF32 Rhs ) const;
	BcVec3d			operator / ( BcF32 Rhs ) const;
	BcVec3d			operator * ( const BcVec3d& Rhs ) const;
	BcVec3d			operator / ( const BcVec3d& Rhs ) const;

	BcVec3d&		operator += ( const BcVec3d& Rhs );
	BcVec3d&		operator -= ( const BcVec3d& Rhs );
	BcVec3d&		operator *= ( BcF32 Rhs );
	BcVec3d&		operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	BcVec3d			operator - () const;
	BcF32			magnitude() const;
	BcF32			magnitudeSquared() const;      
	BcVec3d			normal() const;
	void			normalise();
	BcF32			dot( const BcVec3d& Rhs ) const;
	BcVec3d			cross( const BcVec3d& Rhs ) const;
	BcVec3d			reflect( const BcVec3d& Normal ) const;

	// Interpolation
	void lerp( const BcVec3d& A, const BcVec3d& B, BcF32 T );

	// Comparison with epsilons
	BcBool			operator == (const BcVec3d& Rhs) const;	
	BcBool			operator != (const BcVec3d& Rhs) const;		
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcVec3d::BcVec3d():
	BcVecQuad( 0.0f, 0.0f, 0.0f )
{

}

BcForceInline BcVec3d::BcVec3d( BcF32 X, BcF32 Y, BcF32 Z ):
	BcVecQuad( X, Y, Z )
{

}

BcForceInline BcVec3d::BcVec3d( const BcVec2d& Rhs, BcF32 Z ):
	BcVecQuad( Rhs.x(), Rhs.y(), Z )
{

}


BcForceInline void BcVec3d::set( BcF32 X, BcF32 Y, BcF32 Z )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
}

BcForceInline BcVec3d BcVec3d::operator + ( const BcVec3d& Rhs ) const
{
	return BcVec3d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_ );				
}

BcForceInline BcVec3d BcVec3d::operator - ( const BcVec3d& Rhs ) const
{ 
	return BcVec3d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_ );				
}

BcForceInline BcVec3d BcVec3d::operator * ( BcF32 Rhs ) const
{
	return BcVec3d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs );
}

BcForceInline BcVec3d BcVec3d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return BcVec3d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs);					
}

BcForceInline BcVec3d BcVec3d::operator * ( const BcVec3d& Rhs ) const
{
	return BcVec3d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_ );
}

BcForceInline BcVec3d BcVec3d::operator / ( const BcVec3d& Rhs ) const
{ 
	return BcVec3d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_);					
}

BcForceInline BcVec3d& BcVec3d::operator += ( const BcVec3d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	return (*this);			
}

BcForceInline BcVec3d& BcVec3d::operator -= ( const BcVec3d& Rhs ) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	return (*this);			
}

BcForceInline BcVec3d& BcVec3d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	return (*this);				
}

BcForceInline BcVec3d& BcVec3d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	return (*this);				
}

BcForceInline BcVec3d BcVec3d::operator - () const
{
	return BcVec3d( -X_, -Y_, -Z_ );
}

BcForceInline BcF32 BcVec3d::magnitude() const
{
	return BcSqrt( magnitudeSquared() );
}

BcForceInline BcF32 BcVec3d::magnitudeSquared() const
{
	return ( X_ * X_ ) + ( Y_ * Y_ ) + ( Z_ * Z_ );
}

BcForceInline BcVec3d BcVec3d::normal() const
{
	BcF32 Mag = magnitude();

	if ( Mag == 0.0f )
	{
		return BcVec3d(0,0,0);
	}

	const BcF32 InvMag = 1.0f / Mag;
	return BcVec3d(X_ * InvMag, Y_ * InvMag, Z_ * InvMag);
}

BcForceInline void BcVec3d::normalise()
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
}

BcForceInline BcF32 BcVec3d::dot( const BcVec3d& Rhs ) const
{
	return ( X_ * Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_ * Rhs.Z_ );
}

BcForceInline BcVec3d BcVec3d::cross( const BcVec3d& Rhs ) const
{
	return BcVec3d( ( Y_ * Rhs.Z_)  - ( Rhs.Y_ * Z_ ), ( Z_* Rhs.X_ ) - ( Rhs.Z_ * X_ ), ( X_ * Rhs.Y_ ) - ( Rhs.X_ * Y_ ) );
}

BcForceInline BcBool BcVec3d::operator == (const BcVec3d &Rhs) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) );
}

BcForceInline BcVec3d BcVec3d::reflect( const BcVec3d& Normal ) const
{
	return ( *this - ( Normal * ( 2.0f * this->dot( Normal ) ) ) );
}

BcForceInline BcBool BcVec3d::operator != ( const BcVec3d &Rhs ) const
{
	return !((*this)==Rhs);
}

//////////////////////////////////////////////////////////////////////////
// BcVec4d
class BcVec4d: public BcVecQuad
{
public:
	// Ctors
	BcVec4d(){}
	BcVec4d( const BcVec2d& Rhs );
	BcVec4d( const BcVec3d& Rhs, BcF32 W = 0.0f );
	BcVec4d( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W );
	BcVec4d( const BcChar* pString );

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
	BcVec4d			operator + ( const BcVec4d& Rhs ) const;
	BcVec4d			operator - ( const BcVec4d& Rhs ) const;
	BcVec4d			operator * ( BcF32 Rhs ) const;
	BcVec4d			operator / ( BcF32 Rhs ) const;
	BcVec4d			operator * ( const BcVec4d& Rhs ) const;
	BcVec4d			operator / ( const BcVec4d& Rhs ) const;
	BcVec4d&		operator += ( const BcVec4d& Rhs );
	BcVec4d&		operator -= ( const BcVec4d& Rhs );
	BcVec4d&		operator *= ( BcF32 Rhs );
	BcVec4d&		operator /= ( BcF32 Rhs );

	// Slightly more advanced arithmetic
	BcVec4d			operator - () const;
	BcF32			magnitude() const;
	BcF32			magnitudeSquared() const;
	BcF32			dot( const BcVec4d& Rhs ) const;
	void			normalise();
	void			normalise3();
	BcVec4d			normal() const;
	BcVec3d			normal3() const;

	// Interpolation
	void lerp(const BcVec4d& a, const BcVec4d& b, BcF32 t);

	// Comparison with epsilons
	BcBool			operator == (const BcVec4d& Rhs) const;	
	BcBool			operator != (const BcVec4d& Rhs) const;		      
};

//////////////////////////////////////////////////////////////////////////
// Inlines
BcForceInline BcVec4d::BcVec4d( const BcVec2d& Rhs ):
	BcVecQuad( Rhs.x(), Rhs.y(), 0.0f, 0.0f )
{

}

BcForceInline BcVec4d::BcVec4d( const BcVec3d& Rhs, BcF32 W ):
	BcVecQuad( Rhs.x(), Rhs.y(), Rhs.z(), W )
{

}

BcForceInline BcVec4d::BcVec4d( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W ):
	BcVecQuad( X, Y, Z, W )
{

}

BcForceInline void BcVec4d::set( BcF32 X, BcF32 Y, BcF32 Z, BcF32 W  )
{
	X_ = X;
	Y_ = Y;
	Z_ = Z;
	W_ = W;
}

BcForceInline BcVec4d BcVec4d::operator + ( const BcVec4d& Rhs ) const
{
	return BcVec4d( X_ + Rhs.X_, Y_ + Rhs.Y_, Z_ + Rhs.Z_, W_ + Rhs.W_ );
}

BcForceInline BcVec4d BcVec4d::operator - ( const BcVec4d& Rhs ) const
{ 
	return BcVec4d( X_ - Rhs.X_, Y_ - Rhs.Y_, Z_ - Rhs.Z_, W_ - Rhs.W_ );				
}

BcForceInline BcVec4d BcVec4d::operator * ( BcF32 Rhs ) const
{
	return BcVec4d( X_ * Rhs, Y_ * Rhs, Z_ * Rhs, W_ * Rhs );
}


BcForceInline BcVec4d BcVec4d::operator / ( const BcVec4d& Rhs ) const
{ 
	return BcVec4d( X_ / Rhs.X_, Y_ / Rhs.Y_, Z_ / Rhs.Z_, W_ / Rhs.W_ );					
}

BcForceInline BcVec4d BcVec4d::operator * ( const BcVec4d& Rhs ) const
{
	return BcVec4d( X_ * Rhs.X_, Y_ * Rhs.Y_, Z_ * Rhs.Z_, W_ * Rhs.W_ );
}


BcForceInline BcVec4d BcVec4d::operator / ( BcF32 Rhs ) const
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	return BcVec4d( X_ * InvRhs, Y_ * InvRhs, Z_ * InvRhs, W_ * InvRhs );					
}

BcForceInline BcVec4d& BcVec4d::operator += ( const BcVec4d& Rhs ) 
{ 
	X_ += Rhs.X_;
	Y_ += Rhs.Y_;
	Z_ += Rhs.Z_;
	W_ += Rhs.W_;
	return (*this);			
}

BcForceInline BcVec4d& BcVec4d::operator -= (const BcVec4d& Rhs) 
{ 
	X_ -= Rhs.X_;
	Y_ -= Rhs.Y_;
	Z_ -= Rhs.Z_;
	W_ -= Rhs.W_;
	return (*this);			
}

BcForceInline BcVec4d& BcVec4d::operator *= ( BcF32 Rhs ) 	
{ 
	X_ *= Rhs;
	Y_ *= Rhs;
	Z_ *= Rhs;
	W_ *= Rhs;
	return (*this);				
}

BcForceInline BcVec4d& BcVec4d::operator /= ( BcF32 Rhs ) 	
{ 
	const BcF32 InvRhs = 1.0f / Rhs;
	X_ *= InvRhs;
	Y_ *= InvRhs;
	Z_ *= InvRhs;
	W_ *= InvRhs;
	return (*this);				
}

BcForceInline BcVec4d BcVec4d::operator - () const
{
	return BcVec4d( -X_, -Y_, -Z_, -W_ );
}

BcForceInline BcF32 BcVec4d::magnitude() const
{
	return BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcForceInline BcF32 BcVec4d::magnitudeSquared() const
{
	return ( X_ * X_ + Y_ * Y_ + Z_ * Z_ + W_ * W_ );
}

BcForceInline void BcVec4d::normalise()
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

BcForceInline void BcVec4d::normalise3()
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

BcForceInline BcVec4d BcVec4d::normal() const
{
	BcF32 Mag = magnitude();
	
	if ( Mag == 0.0f )
	{
		return BcVec4d(0,0,0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return BcVec4d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag, W_ * InvMag );
}

BcForceInline BcVec3d BcVec4d::normal3() const
{
	BcF32 Mag = BcSqrt( X_ * X_ + Y_ * Y_ + Z_ * Z_ );
	
	if ( Mag == 0.0f )
	{
		return BcVec3d(0,0,0);
	}
	
	const BcF32 InvMag = 1.0f / Mag;
	return BcVec3d( X_ * InvMag, Y_ * InvMag, Z_ * InvMag );
}


BcForceInline BcF32 BcVec4d::dot( const BcVec4d& Rhs ) const
{
	return ( X_* Rhs.X_ )+( Y_ * Rhs.Y_ )+( Z_* Rhs.Z_ )+( W_ * Rhs.W_ );
}

BcForceInline BcBool BcVec4d::operator == ( const BcVec4d &Rhs ) const
{
	return ( ( BcAbs( X_ - Rhs.X_ ) < BcVecEpsilon ) &&
	         ( BcAbs( Y_ - Rhs.Y_ ) < BcVecEpsilon ) &&
			 ( BcAbs( Z_ - Rhs.Z_ ) < BcVecEpsilon ) &&
			 ( BcAbs( W_ - Rhs.W_ ) < BcVecEpsilon ) );
}

BcForceInline BcBool BcVec4d::operator != ( const BcVec4d &Rhs ) const
{
	return !( (*this) == Rhs );
}

//////////////////////////////////////////////////////////////////////////
// BcCheckFloat
inline BcBool BcCheckFloat( BcVec2d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() );
}

inline BcBool BcCheckFloat( BcVec3d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() ) && BcCheckFloat( T.z() );
}

inline BcBool BcCheckFloat( BcVec4d T )
{
	return BcCheckFloat( T.x() ) && BcCheckFloat( T.y() ) && BcCheckFloat( T.z() ) && BcCheckFloat( T.w() );
}

#endif

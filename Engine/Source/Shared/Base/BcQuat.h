/**************************************************************************
*
* File:		BcQuat.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#include "Base/BcMath.h"


#include "Base/BcTypes.h"
#include "Base/BcVectors.h"
#include "Base/BcMat4d.h"

#ifndef	__BCQUAT_H__
#define __BCQUAT_H__

class BcQuat : public BcVec4d
{
public:
	// ctor	
	BcQuat();
	BcQuat(BcF32 lX, BcF32 lY, BcF32 lZ, BcF32 lW);
	BcQuat( const BcChar* pString );
	
	// Arithmetic
	BcQuat 		operator * (const BcQuat& rhs) const;
	BcQuat 		operator ~ () const;

	// Additional stuff
	void 		makeIdentity();
	BcF32 		magnitude();
	
	// Interpolation
	void lerp( const BcQuat& a, const BcQuat& b, BcF32 t );
	void slerp( const BcQuat& a, const BcQuat& b, BcF32 t );
	
	// Quick handy funk
	void rotateVector( BcVec3d& ) const;

	//
	void fromMatrix4d( const BcMat4d& Mat );
	void asMatrix4d( BcMat4d& Matrix ) const;

	//
	void rotateTo( const BcVec3d& From, const BcVec3d& To );
	void axisAngle( const BcVec3d& Axis, BcF32 Angle );
	
	// 
	void fromEular( BcF32 Yaw, BcF32 Pitch, BcF32 Roll );
	BcVec3d asEular() const;

	//
	void calcFromXYZ();
};

// ctor
inline BcQuat::BcQuat()
{

}

inline BcQuat::BcQuat(BcF32 lX, BcF32 lY, BcF32 lZ, BcF32 lW):
	BcVec4d( lX, lY, lZ, lW )
{

}

// Arithmetic

// Cross Product
inline BcQuat BcQuat::operator * (const BcQuat& rhs) const
{
	return BcQuat( (w() * rhs.x()) + (x() * rhs.w()) + (y() * rhs.z()) - (z() * rhs.y()) ,
	               (w() * rhs.y()) + (y() * rhs.w()) + (z() * rhs.x()) - (x() * rhs.z()) ,
	               (w() * rhs.z()) + (z() * rhs.w()) + (x() * rhs.y()) - (y() * rhs.x()) ,
	               (w() * rhs.w()) - (x() * rhs.x()) - (y() * rhs.y()) - (z() * rhs.z()) );
}

// Inverse
inline BcQuat BcQuat::operator ~ () const
{
	return BcQuat(-x(), -y(), -z(), w());
}

// Additional stuff

// Make an identity quaternion 1.0(0.0, 0.0, 0.0)
inline void BcQuat::makeIdentity()
{
	x( 0.0f );
	y( 0.0f );
	z( 0.0f );
	w( 1.0f );
}

// Return the magnitude
inline BcF32 BcQuat::magnitude()
{
	return BcSqrt(w() * w() + x() * x() + y() * y() + z() * z());
}

// Quick speed up - needs optimising
inline void BcQuat::rotateVector( BcVec3d& vec ) const
{	
	const BcQuat& This = (*this);
	const BcQuat OutVec = This * BcQuat( vec.x(), vec.y(), vec.z(), 1.0f ) * ~This;

	vec.set( OutVec.x(), OutVec.y(), OutVec.z() );       		
}

#endif

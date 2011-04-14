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

#include "BcMath.h"


#include "BcTypes.h"
#include "BcVectors.h"
#include "BcMat4d.h"

#ifndef	__BCQUAT_H__
#define __BCQUAT_H__

class BcQuat : public BcVec4d
{
public:
	// ctor	
	BcQuat();
	BcQuat(BcReal lX, BcReal lY, BcReal lZ, BcReal lW);
	
	// Arithmetic
	BcQuat 		operator * (const BcQuat& rhs) const;
	BcQuat 		operator ~ () const;

	// Additional stuff
	void 		makeIdentity();
	BcReal 		magnitude();
	
	// Interpolation
	void lerp( const BcQuat& a, const BcQuat& b, BcReal t );
	void slerp( const BcQuat& a, const BcQuat& b, BcReal t );
	
	// Quick handy funk
	void rotateVector( BcVec3d& ) const;

	//
	void fromMatrix4d( const BcMat4d& Mat );
	BcMat4d asMatrix4d() const;

	// 
	void fromEular( BcReal Yaw, BcReal Pitch, BcReal Roll );
	BcVec3d asEular() const;

	//
	void calcFromXYZ();
};

// ctor
inline BcQuat::BcQuat()
{

}

inline BcQuat::BcQuat(BcReal lX, BcReal lY, BcReal lZ, BcReal lW):
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
inline BcReal BcQuat::magnitude()
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

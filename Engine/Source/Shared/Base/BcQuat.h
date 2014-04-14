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

#include "Base/BcTypes.h"
#include "Base/BcVec3d.h"
#include "Base/BcVec4d.h"
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
	makeIdentity();
}

inline BcQuat::BcQuat(BcF32 lX, BcF32 lY, BcF32 lZ, BcF32 lW):
	BcVec4d( lX, lY, lZ, lW )
{

}

#endif

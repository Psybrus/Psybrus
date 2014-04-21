/**************************************************************************
*
* File:		MaQuat.h
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
#include "Math/MaVec3d.h"
#include "Math/MaVec4d.h"
#include "Math/MaMat4d.h"

#ifndef	__BCQUAT_H__
#define __BCQUAT_H__

class MaQuat : public MaVec4d
{
public:
	// ctor	
	MaQuat();
	MaQuat(BcF32 lX, BcF32 lY, BcF32 lZ, BcF32 lW);
	MaQuat( const BcChar* pString );
	
	// Arithmetic
	MaQuat 		operator * (const MaQuat& rhs) const;
	MaQuat 		operator ~ () const;

	// Additional stuff
	void 		makeIdentity();
	BcF32 		magnitude();
	
	// Interpolation
	void lerp( const MaQuat& a, const MaQuat& b, BcF32 t );
	void slerp( const MaQuat& a, const MaQuat& b, BcF32 t );
	
	// Quick handy funk
	void rotateVector( MaVec3d& ) const;

	//
	void fromMatrix4d( const MaMat4d& Mat );
	void asMatrix4d( MaMat4d& Matrix ) const;

	//
	void rotateTo( const MaVec3d& From, const MaVec3d& To );
	void axisAngle( const MaVec3d& Axis, BcF32 Angle );
	
	// 
	void fromEular( BcF32 Yaw, BcF32 Pitch, BcF32 Roll );
	MaVec3d asEular() const;

	//
	void calcFromXYZ();
};

// ctor
inline MaQuat::MaQuat()
{
	makeIdentity();
}

inline MaQuat::MaQuat(BcF32 lX, BcF32 lY, BcF32 lZ, BcF32 lW):
	MaVec4d( lX, lY, lZ, lW )
{

}

#endif

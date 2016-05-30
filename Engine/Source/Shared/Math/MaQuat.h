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
#include "Reflection/ReReflection.h"

#ifndef	__BCQUAT_H__
#define __BCQUAT_H__

class MaQuat : public MaVec4d
{
public:
	REFLECTION_DECLARE_BASIC( MaQuat );

public:
	// ctor	
	MaQuat();
	MaQuat(BcF32 lX, BcF32 lY, BcF32 lZ, BcF32 lW);
	MaQuat( const BcChar* pString );
	
	// Arithmetic
	MaQuat operator * (const MaQuat& rhs) const;
	MaQuat operator ~ () const;


	// Additional stuff
	void makeIdentity();
	BcF32 magnitude();
	void inverse();
	
	// Interpolation
	void lerp( const MaQuat& a, const MaQuat& b, BcF32 t );
	void slerp( const MaQuat& a, const MaQuat& b, BcF32 t );
	
	MaVec3d rotateVector( const MaVec3d& ) const;

	void fromAxis( const MaVec3d& X, const MaVec3d& Y, const MaVec3d& Z );

	//
	void fromMatrix4d( const MaMat4d& Mat );
	void asMatrix4d( MaMat4d& Matrix ) const;

	//
	void rotateTo( const MaVec3d& From, const MaVec3d& To );
	void axisAngle( const MaVec3d& Axis, BcF32 Angle );
	
	// 
	void fromEuler( BcF32 Yaw, BcF32 Pitch, BcF32 Roll );
	MaVec3d asEuler() const;

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

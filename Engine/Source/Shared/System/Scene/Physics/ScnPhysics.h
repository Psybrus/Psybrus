/**************************************************************************
*
* File:		ScnPhysics.h
* Author:	Neil Richardson 
* Ver/Date:	25/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNPHYSICS_H__
#define __SCNPHYSICS_H__

#include "Math/MaVec3d.h"
#include "Math/MaMat4d.h"
#include "Math/MaQuat.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

//////////////////////////////////////////////////////////////////////////
// FromBullet - 3D Vector
BcForceInline MaVec3d ScnPhysicsFromBullet( const btVector3& From )
{
	return MaVec3d( From.x(), From.y(), From.z() );
}

//////////////////////////////////////////////////////////////////////////
// ToBullet - 3D Vector.
BcForceInline btVector3 ScnPhysicsToBullet( const MaVec3d& From )
{
	return btVector3( From.x(), From.y(), From.z() );
}

//////////////////////////////////////////////////////////////////////////
// FromBullet - Quaternion
BcForceInline MaQuat ScnPhysicsFromBullet( const btQuaternion& From )
{
	return MaQuat( From.x(), From.y(), From.z(), From.w() );
}

//////////////////////////////////////////////////////////////////////////
// ToBullet - Quaternion
BcForceInline btQuaternion ScnPhysicsToBullet( const MaQuat& From )
{
	return btQuaternion( From.x(), From.y(), From.z(), From.w() );
}

//////////////////////////////////////////////////////////////////////////
// FromBullet - Transform.
BcForceInline MaMat4d ScnPhysicsFromBullet( const btTransform& From )
{
	MaMat4d To;
	From.getOpenGLMatrix( reinterpret_cast< btScalar* >( &To ) );
	return To;
}

//////////////////////////////////////////////////////////////////////////
// ToBullet - Transform.
BcForceInline btTransform ScnPhysicsToBullet( const MaMat4d& From )
{
	btTransform To;
	To.setFromOpenGLMatrix( reinterpret_cast< const btScalar* >( &From ) );
	return To;
}


#endif

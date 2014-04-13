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

#include "Base/BcVec3d.h"
#include "Base/BcMat4d.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

//////////////////////////////////////////////////////////////////////////
// FromBullet - 3D Vector
BcForceInline BcVec3d ScnPhysicsFromBullet( const btVector3& From )
{
	return BcVec3d( From.x(), From.y(), From.z() );
}

//////////////////////////////////////////////////////////////////////////
// ToBullet - 3D Vector.
BcForceInline btVector3 ScnPhysicsToBullet( const BcVec3d& From )
{
	return btVector3( From.x(), From.y(), From.z() );
}

//////////////////////////////////////////////////////////////////////////
// FromBullet - Transform.
BcForceInline BcMat4d ScnPhysicsFromBullet( const btTransform& From )
{
	BcMat4d To;
	From.getOpenGLMatrix( reinterpret_cast< btScalar* >( &To ) );
	return To;
}

//////////////////////////////////////////////////////////////////////////
// ToBullet - Transform.
BcForceInline btTransform ScnPhysicsToBullet( const BcMat4d& From )
{
	btTransform To;
	To.setFromOpenGLMatrix( reinterpret_cast< const btScalar* >( &From ) );
	return To;
}


#endif

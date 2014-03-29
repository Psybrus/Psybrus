/**************************************************************************
*
* File:		ScnPhysicsFileData.h
* Author:	Neil Richardson 
* Ver/Date:	25/02/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNPHYSICSFILEDATA__
#define __SCNPHYSICSFILEDATA__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsCollisionShapeHeader
struct ScnPhysicsCollisionShapeHeader
{

};

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsBoxCollisionShapeHeader
struct ScnPhysicsBoxCollisionShapeHeader:
	public ScnPhysicsCollisionShapeHeader
{
	BcVec3d HalfExtents_;
};

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsSphereCollisionShapeHeader
struct ScnPhysicsSphereCollisionShapeHeader:
	public ScnPhysicsCollisionShapeHeader
{
	BcF32 Radius_;
};

#endif

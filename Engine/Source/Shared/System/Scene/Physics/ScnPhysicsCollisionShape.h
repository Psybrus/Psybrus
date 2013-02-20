/**************************************************************************
*
* File:		ScnPhysicsCollisionShape.h
* Author:	Neil Richardson 
* Ver/Date:	20/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsCollisionShape_H__
#define __ScnPhysicsCollisionShape_H__

#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsCollisionShape
class ScnPhysicsCollisionShape:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnPhysicsCollisionShape );
	

	virtual void							initialise();
	virtual void							create();
	virtual void							destroy();

	class btCollisionShape*					getCollisionShape();


protected:
	class btCollisionShape*					CollisionShape_;
};

#endif

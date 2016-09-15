/**************************************************************************
*
* File:		ScnPhysicsMeshCollisionComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsMeshCollisionComponent_H__
#define __ScnPhysicsMeshCollisionComponent_H__

#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsMeshCollisionComponent
class ScnPhysicsMeshCollisionComponent:
	public ScnPhysicsCollisionComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsMeshCollisionComponent, ScnPhysicsCollisionComponent );
	
	ScnPhysicsMeshCollisionComponent();
	ScnPhysicsMeshCollisionComponent( class ScnPhysicsMesh* Mesh, MaVec3d Size, BcF32 Margin );
	virtual ~ScnPhysicsMeshCollisionComponent();

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

protected:
	class ScnPhysicsMesh* Mesh_;
	MaVec3d Size_;
	BcF32 Margin_;
};

#endif

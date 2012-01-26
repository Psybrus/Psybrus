/**************************************************************************
*
* File:		ScnRigidBodyWorld.h
* Author:	Neil Richardson 
* Ver/Date:	21/01/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnRigidBodyWorld_H__
#define __ScnRigidBodyWorld_H__

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnRigidBody.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelRef
typedef CsResourceRef< class ScnRigidBodyWorld > ScnRigidBodyWorldRef;

//////////////////////////////////////////////////////////////////////////
// ScnModelComponentRef
typedef CsResourceRef< class ScnRigidBodyWorldComponent > ScnRigidBodyWorldComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnRigidBodyWorld
class ScnRigidBodyWorld:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnRigidBodyWorld );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
private:
	void								setup();
	
private:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:
	friend class ScnRigidBodyWorldComponent;
		
	// Header.
	struct THeader
	{
	};
	
	THeader*							pHeader_;
};

//////////////////////////////////////////////////////////////////////////
// ScnRigidBodyWorldComponent
class ScnRigidBodyWorldComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnRigidBodyWorldComponent );
		
	static ScnRigidBodyWorldComponentRef StaticGetComponent();

	virtual void						initialise( ScnRigidBodyWorldRef Parent );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();

	void								addRigidBodyComponent( ScnRigidBodyComponentRef RigidBodyComponent );
	void								removeRigidBodyComponent( ScnRigidBodyComponentRef RigidBodyComponent );

	BcBool								lineCheck( const BcVec3d& Start, const BcVec3d& End, BcVec3d& Intersection );
	
public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
public: // NEILO HACK.
	friend class ScnModelComponent;

	static ScnRigidBodyWorldComponent*		pStaticComponent_;
	
	ScnRigidBodyWorldRef					Parent_;
	
	class btBroadphaseInterface*			pBroadphase_;
	class btCollisionDispatcher*			pDispatcher_;
	class btConstraintSolver*				pSolver_;
	class btDefaultCollisionConfiguration*	pCollisionConfiguration_;
	class btDynamicsWorld*					pDynamicsWorld_;
};

#endif

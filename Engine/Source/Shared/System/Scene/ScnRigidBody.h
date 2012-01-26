/**************************************************************************
*
* File:		ScnRigidBody.h
* Author:	Neil Richardson 
* Ver/Date:	21/01/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNRIGIDBODY_H__
#define __SCNRIGIDBODY_H__

#include "RsCore.h"
#include "CsResourceRef.h"

#include "ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelRef
typedef CsResourceRef< class ScnRigidBody > ScnRigidBodyRef;

//////////////////////////////////////////////////////////////////////////
// ScnModelComponentRef
typedef CsResourceRef< class ScnRigidBodyComponent > ScnRigidBodyComponentRef;

//////////////////////////////////////////////////////////////////////////
// ScnRigidBody
class ScnRigidBody:
	public CsResource
{
public:
	DECLARE_RESOURCE( CsResource, ScnRigidBody );
	
#if PSY_SERVER
	virtual BcBool						import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif
	
	virtual void						initialise();
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
private:
	void								setup();
	
protected:
	void								fileReady();
	void								fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:
	friend class ScnRigidBodyComponent;
	
	enum TShapeType
	{
		ST_MESH = 0,
		ST_SPHERE,
		ST_BOX,
		ST_CAPSULE,

		ST_MAX,
		ST_FORCE_DWORD
	};
	
	// Header.
	struct THeader
	{
		// Shape.
		TShapeType						Type_;
		BcVec3d							Extents_;
		BcU32							NoofVertices_;
		BcU32							NoofIndices_;

		// Body.
		BcReal							Mass_;
		BcReal							E_;
		BcReal							F_;
	};
	
	THeader*							pHeader_;

	BcF32*								pVertexBufferData_;
	int*								pIndexBufferData_;

	class btTriangleIndexVertexArray*	pTriangleIndexVertexArray_;
};

//////////////////////////////////////////////////////////////////////////
// ScnRigidBodyComponent
class ScnRigidBodyComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnRigidBodyComponent );
		
	virtual void						initialise( ScnRigidBodyRef Parent );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	
public:
	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
protected:
	friend class ScnRigidBody;
	friend class ScnRigidBodyWorldComponent;
	
	ScnRigidBodyRef						Parent_;

	class btCollisionShape*				pCollisionShape_;
	class btRigidBody*					pRigidBody_;
};

#endif

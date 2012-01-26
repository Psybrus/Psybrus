/**************************************************************************
*
* File:		ScnRigidBodyWorld.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnRigidBodyWorld.h"
#include "ScnEntity.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#include "Mdl.h"
#endif

// Bullet includes.
#include "btBulletDynamicsCommon.h"

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnRigidBodyWorld::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	BcStream HeaderStream;
	
	//
	pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
	
	//
	return BcTrue;
}

#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRigidBodyWorld );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnRigidBodyWorld::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnRigidBodyWorld" )
		.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRigidBodyWorld::initialise()
{
	// NULL internals.
	pHeader_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRigidBodyWorld::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnRigidBodyWorld::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnRigidBodyWorld::isReady()
{
	return pHeader_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// setup
void ScnRigidBodyWorld::setup()
{

}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnRigidBodyWorld::fileReady()
{
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnRigidBodyWorld::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
		getChunk( ++ChunkIdx );
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( ScnRigidBodyWorldComponent );

ScnRigidBodyWorldComponent* ScnRigidBodyWorldComponent::pStaticComponent_ = NULL;

//////////////////////////////////////////////////////////////////////////
// StaticGetComponent
ScnRigidBodyWorldComponentRef ScnRigidBodyWorldComponent::StaticGetComponent()
{
	return pStaticComponent_;
}

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnRigidBodyWorldComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	PropertyTable.beginCatagory( "ScnRigidBodyWorldComponent" )
		//.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRigidBodyWorldComponent::initialise( ScnRigidBodyWorldRef Parent )
{
	BcAssert( pStaticComponent_ == NULL );
	pStaticComponent_ = this;

	// Cache parent.
	Parent_ = Parent;

	// Setup dynamics world.
	pCollisionConfiguration_ = new btDefaultCollisionConfiguration();
	pDispatcher_ = new	btCollisionDispatcher( pCollisionConfiguration_ );
	pBroadphase_ = new btDbvtBroadphase();
	pSolver_ = new btSequentialImpulseConstraintSolver();
	pDynamicsWorld_ = new btDiscreteDynamicsWorld( pDispatcher_, pBroadphase_, pSolver_, pCollisionConfiguration_ );
	pDynamicsWorld_->setGravity( btVector3( 0.0f, -10.0f, 0.0f ) );
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRigidBodyWorldComponent::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnRigidBodyWorldComponent::destroy()
{
	BcAssert( pStaticComponent_ == this );
	pStaticComponent_ = NULL;

	delete pBroadphase_;
	delete pDispatcher_;
	delete pSolver_;
	delete pCollisionConfiguration_;
	delete pDynamicsWorld_;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnRigidBodyWorldComponent::isReady()
{
	return BcTrue;//Parent_->isReady();
}

//////////////////////////////////////////////////////////////////////////
// addRigidBodyComponent
void ScnRigidBodyWorldComponent::addRigidBodyComponent( ScnRigidBodyComponentRef RigidBodyComponent )
{
	pDynamicsWorld_->addRigidBody( RigidBodyComponent->pRigidBody_ );
}

//////////////////////////////////////////////////////////////////////////
// removeRigidBodyComponent
void ScnRigidBodyWorldComponent::removeRigidBodyComponent( ScnRigidBodyComponentRef RigidBodyComponent )
{
	pDynamicsWorld_->removeRigidBody( RigidBodyComponent->pRigidBody_ );
}

//////////////////////////////////////////////////////////////////////////
// lineCheck
class LocalResultCallback: public btCollisionWorld::RayResultCallback
{
public:
	LocalResultCallback()
	{
	}

	virtual ~LocalResultCallback()
	{

	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
	{
		if( rayResult.m_hitFraction < Closest_ )
		{
			Closest_ = rayResult.m_hitFraction;
			Hit_ = Start_ + ( End_ - Start_ ) * rayResult.m_hitFraction;
			HasHit_ = BcTrue;
		}

		return 0.0f;
	}

	btVector3 Start_;
	btVector3 End_;
	BcBool HasHit_;
	BcReal Closest_;
	btVector3 Hit_;
};

BcBool ScnRigidBodyWorldComponent::lineCheck( const BcVec3d& Start, const BcVec3d& End, BcVec3d& Intersection )
{
	LocalResultCallback ResultCallback;

	ResultCallback.Start_ = btVector3( Start.x(), Start.y(), Start.z() );
	ResultCallback.End_ = btVector3( End.x(), End.y(), End.z() );
	ResultCallback.HasHit_ = BcFalse;
	ResultCallback.Closest_ = 1.0f;

	// Set intersection to end by default.
	Intersection = End;	

	// Do ray test with Bullet.
	pDynamicsWorld_->rayTest( ResultCallback.Start_, ResultCallback.End_, ResultCallback );
	if( ResultCallback.HasHit_ )
	{
		Intersection = BcVec3d( ResultCallback.Hit_.x(), ResultCallback.Hit_.y(), ResultCallback.Hit_.z() );
	}

	//
	return ResultCallback.HasHit_;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnRigidBodyWorldComponent::update( BcReal Tick )
{
	// Step simulation.
	pDynamicsWorld_->stepSimulation( Tick, 0 );
	
	//
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnRigidBodyWorldComponent::onAttach( ScnEntityWeakRef Parent )
{
	//
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnRigidBodyWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
	//
	Super::onDetach( Parent );
}

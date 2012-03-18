/**************************************************************************
*
* File:		ScnEntity.h
* Author:	Neil Richardson 
* Ver/Date:	26/11/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnEntity_H__
#define __ScnEntity_H__

#include "System/Renderer/RsCore.h"
#include "System/Content/CsResource.h"

#include "System/Scene/ScnTypes.h"

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnRenderableComponent.h"

#include "System/Scene/ScnTransform.h"
#include "System/Scene/ScnSpatialTree.h"
#include "System/Scene/ScnVisitor.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntity
class ScnEntity:
	public CsResource,
	public EvtPublisher
{
public:
	DECLARE_RESOURCE( CsResource, ScnEntity );
	
#if PSY_SERVER
	BcBool								import( const Json::Value& Object, CsDependancyList& DependancyList );
#endif	
	void								initialise();
	void								create();
	void								destroy();
	BcBool								isReady();
	
public:
	void								update( BcReal Tick );
	void								render( RsFrame* pFrame, RsRenderSort Sort ); // NEILO TODO: Don't implement here. Test code only.
	void								attach( ScnComponent* Component );
	void								detach( ScnComponent* Component );
	void								reattach( ScnComponent* Component );

	/**
	 * Called when attached to the scene.
	 */
	void								onAttachScene();

	/**
	 * Called when detached from the scene.
	 */
	void								onDetachScene();

	/**
	 * Are we attached to the scene?
	 */
	BcBool								isAttached() const;

	/**
	 * Get number of components.
	 */
	BcU32								getNoofComponents() const;
	
	/**
	 * Get component.
	 */
	ScnComponentRef						getComponent( BcU32 Idx );

	/**
 	 * Get AABB which encompasses this entity.
	 */
	const BcAABB&						getAABB() const;

	/**
	 * Set position.
	 */
	void								setPosition( const BcVec3d& Position );

	/**
	 * Set rotation.
	 */
	void								setRotation( const BcQuat& Rotation );

	/**
	 * Set matrix.
	 */
	void								setMatrix( const BcMat4d& Matrix );

	/**
	 * Get transform.
	 */
	const ScnTransform&					getTransform() const;

protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
protected:
	void								processAttachDetach();
	void								internalAttach( ScnComponent* Component );
	void								internalDetach( ScnComponent* Component );

protected:
	struct THeader
	{
	};
	
	THeader*							pHeader_;

	ScnComponentList					Components_;
	ScnTransform						Transform_;

	ScnComponentList					AttachComponents_;
	ScnComponentList					DetachComponents_;

private:
	BcBool								IsAttached_;
};

#endif

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
#include "System/Scene/ScnViewComponent.h"

#include "System/Scene/ScnTransform.h"
#include "System/Scene/ScnSpatialTree.h"
#include "System/Scene/ScnVisitor.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntity
class ScnEntity:
	public ScnRenderableComponent,
	public EvtPublisher
{
public:
	DECLARE_RESOURCE( ScnRenderableComponent, ScnEntity );
	
#if PSY_SERVER
	BcBool								import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	void								initialise();
	void								initialise( ScnEntityRef Basis );
	void								create();
	void								destroy();
	BcBool								isReady();
	
public:
	void								update( BcReal Tick );
	void								render( ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort ); // NEILO TODO: Don't implement here. Test code only.
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
	 * Get basis entity.
	 */
	ScnEntityRef						getBasisEntity();

	/**
	 * Get number of components.
	 */
	BcU32								getNoofComponents() const;
	
	/**
	 * Get component.
	 */
	ScnComponentRef						getComponent( BcU32 Idx, const BcName& Type = BcName::INVALID );

	/**
	 * Get component by type.
	 */
	template< typename _Ty >
	ScnComponentRef						getComponentByType( BcU32 Idx )
	{
		return getComponent( Idx, _Ty::StaticGetType() );
	}

	/**
	 * 
	 */

	/**
 	 * Get AABB which encompasses this entity.
	 */
	BcAABB								getAABB();

	/**
	 * Set position.
	 */
	void								setPosition( const BcVec3d& Position );

	/**
	 * Set matrix.
	 */
	void								setMatrix( const BcMat4d& Matrix );

	/**
	 * Get matrix.
	 */
	const BcMat4d&						getMatrix() const;


protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	void								processAttachDetach();
	void								internalAttach( ScnComponent* Component );
	void								internalDetach( ScnComponent* Component );

protected:
	const BcChar*						pJsonObject_; // TEMP.

	ScnEntityRef						Basis_;
	BcMat4d								Transform_;

	ScnComponentList					Components_;
	ScnComponentList					AttachComponents_;
	ScnComponentList					DetachComponents_;

private:
	BcBool								IsAttached_;
};

#endif

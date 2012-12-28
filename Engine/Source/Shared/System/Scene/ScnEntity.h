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

#include "Events/EvtProxyBuffered.h"

#include "System/Scene/ScnTypes.h"

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnRenderableComponent.h"
#include "System/Scene/ScnViewComponent.h"

#include "System/Scene/ScnSpatialTree.h"
#include "System/Scene/ScnVisitor.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntity
class ScnEntity:
	public ScnComponent,
	public EvtPublisher
{
public:
	DECLARE_RESOURCE( ScnComponent, ScnEntity );
	
#if PSY_SERVER
	BcBool								import( class CsPackageImporter& Importer, const Json::Value& Object );
#endif	
	void								initialise();
	void								initialise( ScnEntityRef Basis );
	void								create();
	void								destroy();

public:
	void								update( BcF32 Tick );
	void								attach( ScnComponent* Component );
	void								detach( ScnComponent* Component );
	void								onAttach( ScnEntityWeakRef Parent );
	void								onDetach( ScnEntityWeakRef Parent );
	void								detachFromParent();
	
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
	 * Get component.
	 */
	ScnComponentRef						getComponent( BcName Name, const BcName& Type = BcName::INVALID );

	/**
	 * Get component by type.
	 */
	template< typename _Ty >
	ScnComponentRef						getComponentByType( BcU32 Idx )
	{
		return getComponent( Idx, _Ty::StaticGetType() );
	}

	/**
	 * Get component by type.
	 */
	template< typename _Ty >
	ScnComponentRef						getComponentByType( BcName Name )
	{
		return getComponent( Name, _Ty::StaticGetType() );
	}

	/**
	 * Get component on any parent or self.
	 */
	ScnComponentRef						getComponentAnyParent( BcU32 Idx, const BcName& Type = BcName::INVALID );

	/**
	 * Get component on any parent or self by type.
	 */
	template< typename _Ty >
	ScnComponentRef						getComponentAnyParentByType( BcU32 Idx )
	{
		return getComponentAnyParent( Idx, _Ty::StaticGetType() );
	}

	/**
	 * Set position.
	 */
	void								setPosition( const BcVec3d& Position );

	/**
	 * Set matrix.
	 */
	void								setMatrix( const BcMat4d& Matrix );

	/**
	 * Get position.
	 */
	BcVec3d								getPosition() const;

	/**
	 * Get matrix.
	 */
	const BcMat4d&						getMatrix() const;

protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	const BcChar*						pJsonObject_; // TEMP.

	ScnEntityRef						Basis_;
	BcMat4d								Transform_;

	ScnComponentList					Components_;
	
	EvtProxyBuffered*					pEventProxy_;
};

#endif

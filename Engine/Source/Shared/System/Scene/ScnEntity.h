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

#include "System/Content/CsResource.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnComponent.h"

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
	ScnComponent*						getComponent( BcU32 Idx = 0, const ReClass* Class = nullptr );

	/**
	 * Get component.
	 */
	ScnComponent*						getComponent( BcName Name, const ReClass* Class = nullptr );

	/**
	 * Get component by type.
	 */
	template< typename _Ty >
	_Ty*								getComponentByType( BcU32 Idx = 0 )
	{
		return static_cast< _Ty* >( getComponent( Idx, _Ty::StaticGetClass() ) );
	}

	/**
	 * Get component by type.
	 */
	template< typename _Ty >
	_Ty*								getComponentByType( const ReClass* Class )
	{
		return static_cast< _Ty* >( getComponent( 0, _Ty::StaticGetClass() ) );
	}

	/**
	 * Get component on any parent or self.
	 */
	ScnComponent*						getComponentAnyParent( BcU32 Idx = 0, const ReClass* Class = nullptr );

	/**
	 * Get component on any parent or self.
	 */
	ScnComponent*						getComponentAnyParent( BcName Name, const ReClass* Class = nullptr );

	/**
	 * Get component on any parent or self by type.
	 */
	template< typename _Ty >
	_Ty*								getComponentAnyParentByType( BcU32 Idx = 0 )
	{
		return static_cast< _Ty* >( getComponentAnyParent( Idx, _Ty::StaticGetClass() ) );
	}

	/**
	 * Get component on any parent or self by type.
	 */
	template< typename _Ty >
	_Ty*								getComponentAnyParentByType( BcName Name )
	{
		return static_cast< _Ty* >( getComponentAnyParent( Name, _Ty::StaticGetClass() ) );
	}

	/**
	 * Set position.
	 */
	void								setLocalPosition( const BcVec3d& Position );

	/**
	 * Set matrix.
	 */
	void								setLocalMatrix( const BcMat4d& Matrix );

	/**
	 * Get local position.
	 */
	BcVec3d								getLocalPosition() const;

	/**
	 * Get position.
	 */
	BcVec3d								getWorldPosition() const;

	/**
	 * Get matrix.
	 */
	const BcMat4d&						getLocalMatrix() const;

	/**
	 * Get matrix.
	 */
	const BcMat4d&						getWorldMatrix() const;

protected:
	virtual void						fileReady();
	virtual void						fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	const BcChar*						pJsonObject_; // TEMP.

	ScnEntityRef						Basis_;
	BcMat4d								LocalTransform_;
	BcMat4d								WorldTransform_;

	ScnComponentList					Components_;
	
	class EvtProxyBuffered*				pEventProxy_;
};

#endif

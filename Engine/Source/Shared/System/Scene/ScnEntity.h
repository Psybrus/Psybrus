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

#define SCNENTITY_USES_EVTPUBLISHER ( 0 )

//////////////////////////////////////////////////////////////////////////
// ScnEntityHeader
struct ScnEntityHeader
{
	BcU32 NoofComponents_;
};

//////////////////////////////////////////////////////////////////////////
// ScnEntity
class ScnEntity:
	public ScnComponent
#if SCNENTITY_USES_EVTPUBLISHER
	, public EvtPublisher
#endif
	{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnEntity, ScnComponent );
	
	ScnEntity();
	ScnEntity( ReNoInit );
	virtual ~ScnEntity();

	void initialise();
	void initialise( ScnEntityRef Basis );

public:
	void update( BcF32 Tick );
	void attach( ScnComponent* Component );
	void detach( ScnComponent* Component );
	void onAttach( ScnEntityWeakRef Parent );
	void onDetach( ScnEntityWeakRef Parent );
	void detachFromParent();
	
	/**
	 * Get basis entity.
	 */
	ScnEntityRef getBasisEntity();

	/**
	 * Get number of components.
	 */
	BcU32 getNoofComponents() const;
	
	/**
	 * Get component.
	 */
	ScnComponent* getComponent( BcU32 Idx = 0, const ReClass* Class = nullptr ) override;

	/**
	 * Get component.
	 */
	ScnComponent* getComponent( BcName Name, const ReClass* Class = nullptr ) override;

	/**
	 * Get component on any parent or self.
	 */
	ScnComponent* getComponentAnyParent( BcU32 Idx = 0, const ReClass* Class = nullptr ) override;

	/**
	 * Get component on any parent or self.
	 */
	ScnComponent* getComponentAnyParent( BcName Name, const ReClass* Class = nullptr ) override;

	/**
	 * Set position.
	 */
	void setLocalPosition( const MaVec3d& Position );

	/**
	 * Set matrix.
	 */
	void setLocalMatrix( const MaMat4d& Matrix );

	/**
	 * Set matrix rot + scale part.
	 */
	void setLocalMatrixRS( const MaMat4d& Matrix );

	/**
	 * Get local position.
	 */
	MaVec3d getLocalPosition() const;

	/**
	 * Set position.
	 */
	void setWorldPosition( const MaVec3d& Position );

	/**
	 * Set matrix.
	 */
	void setWorldMatrix( const MaMat4d& Matrix );

	/**
	 * Set matrix rot + scale part.
	 */
	void setWorldMatrixRS( const MaMat4d& Matrix );

	/**
	 * Get position.
	 */
	MaVec3d getWorldPosition() const;

	/**
	 * Get matrix.
	 */
	const MaMat4d& getLocalMatrix() const;

	/**
	 * Get matrix.
	 */
	const MaMat4d& getWorldMatrix() const;

protected:
	virtual void fileReady();
	virtual void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

	void setupComponents();
	
protected:
	const ScnEntityHeader* pHeader_;

	MaMat4d LocalTransform_;
	MaMat4d WorldTransform_;

	ScnComponentList Components_;
	
#if SCNENTITY_USES_EVTPUBLISHER
	class EvtProxyBuffered* pEventProxy_;
#endif
};

#endif

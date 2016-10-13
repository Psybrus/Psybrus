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

#include "Events/EvtPublisher.h"

#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnComponent.h"

#define SCNENTITY_USES_EVTPUBLISHER ( 1 )

//////////////////////////////////////////////////////////////////////////
// ScnEntityHeader
struct ScnEntityHeader
{
	MaMat4d LocalTransform_;
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

	void initialise() override;
	void destroy() override;

	void visitHierarchy( 
		ScnComponentVisitType VisitType, 
		ScnEntity* Parent,
		const ScnComponentVisitFunc& Func ) override;

public:
	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	/**
	 * Attach a component to this entity.
	 * @pre Component is not nullptr.
	 * @pre Component is not attached to another entity.
	 * @post Component is stored in component list.
	 * @post Component is queued for pending onAttach.
	 */
	void attach( ScnComponent* Component );

	/**
	 * Attach a new component to this enitty.
	 * @post Component is stored in component list.
	 * @post Component is queued for pending onAttach.
	 */
	template< typename _Ty, typename... _ParamT >
	_Ty* attach( const BcName& Name, _ParamT... Params );

	/**
	 * Detach a component from this entity.
	 * @post Component is stored in component list.
	 * @post Component is queued for pending onDetach.
	 */
	void detach( ScnComponent* Component );

	/**
	 * Detach all components from this entity.
	 * @post All components will be detached recursively.
	 */
	void detachAll();
	
	/**
	 * Get basis entity.
	 */
	ScnEntityRef getBasisEntity();

	/**
	 * Get number of components.
	 */
	size_t getNoofComponents() const;

	/**
	 * Get components.
	 */
	const ScnComponentList& getComponents() const;
	
	/**
	 * Get component.
	 */
	ScnComponent* getComponent( size_t Idx = 0, const ReClass* Class = nullptr ) override;

	/**
	 * Get component.
	 */
	ScnComponent* getComponent( BcName Name, const ReClass* Class = nullptr ) override;

	/**
	 * Get component on any parent or self.
	 */
	ScnComponent* getComponentAnyParent( size_t Idx = 0, const ReClass* Class = nullptr ) override;

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
	/**
	 * @pre Class is not nullptr.
	 * @pre Class is a valid class.
	 * @post Will always return a valid component.
	 */
	ScnComponent* internalCreateComponent( const BcName& Name, const ReClass* Class );

	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;

	void setupComponents();

private:
	static void update( const ScnComponentList& Components );

private:
	const ScnEntityHeader* pHeader_;

	MaMat4d LocalTransform_;
	MaMat4d WorldTransform_;

	ScnComponentList Components_;
	
#if SCNENTITY_USES_EVTPUBLISHER
	class EvtProxyBuffered* pEventProxy_;
#endif
};

//////////////////////////////////////////////////////////////////////////
// Inlines
template< typename _Ty, typename... _ParamT >
BcForceInline _Ty* ScnEntity::attach( const BcName& Name, _ParamT... Params )
{
	BcName UniqueName = Name.isValid() ? Name : BcName( _Ty::StaticGetClass()->getName() ).getUnique();
	auto Component = new _Ty( Params... );
	Component->setName( UniqueName );
	Component->setOwner( getPackage() );
	Component->initialise();
	Component->postInitialise();
	attach( Component );
	return Component;
}


#endif

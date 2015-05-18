/**************************************************************************
*
* File:		ScnCore.h
* Author:	Neil Richardson 
* Ver/Date:	23/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnCore_H__
#define __ScnCore_H__

#include "Base/BcTypes.h"
#include "Base/BcGlobal.h"

#include "System/SysSystem.h"

#include "System/Scene/ScnEntity.h"
#include "System/ScnComponentProcessor.h"

//////////////////////////////////////////////////////////////////////////
// ScnCore
class ScnCore:
	public SysSystem,
	public ScnComponentProcessRegistry,
	public BcGlobal< ScnCore >
{
public:
	ScnCore();
	virtual ~ScnCore();

	virtual void open();
	virtual void update();
	virtual void close();

	/**
	 * Adds entity into the scene and ready to recieve updates.
	 */
	void addEntity( ScnEntityRef Entity );

	/**
	 * Remove entity from the scene so it no longer recieves updates.
	 */
	void removeEntity( ScnEntityRef Entity );

	/**
	 * Add callback.
	 */
	void addCallback( class ScnCoreCallback* Callback );

	/**
	 * Remove callback.
	 */
	void removeCallback( class ScnCoreCallback* Callback );

	/**
	 * Remove all entities from the scene.
	 */
	void removeAllEntities();

	/**
	 * Create an entity from template.
	 */
	ScnEntityRef createEntity( const BcName& Package, const BcName& Name, const BcName& InstanceName = BcName::INVALID );

	/**
	 * Spawn an entity from template. Handles loading and scene attachment.
	 * Will return NULL if async.
	 */
	ScnEntity* spawnEntity( const ScnEntitySpawnParams& Params );

	/**
	 * Find an entity. Non recursive, only searching within the manager, not parented entities.
	 */
	ScnEntityRef findEntity( const BcName& InstanceName );

	/**
	 * Get an entity.
	 */
	ScnEntityRef getEntity( BcU32 Idx );

	/**
	 * Queue component for attach.
	 */
	void queueComponentForAttach( ScnComponentRef Component );

	/**
	 * Queue component for detach.
	 */
	void queueComponentForDetach( ScnComponentRef Component );

	/**
	 * Visit view.
	 */
	void visitView( class ScnVisitor* pVisitor, const class ScnViewComponent* View );

	/**
	 * Visit bounds.
	 */
	void visitBounds( class ScnVisitor* pVisitor, const MaAABB& Bounds );

	/**
	 * Get component list.
	 */
	template < class _Ty >
	const ScnComponentList& getComponentList() const;

private:
	/**
	 * Called when a component has been attached to an entity that exists in the scene.
	 */
	void onAttachComponent( ScnEntityWeakRef Entity, ScnComponent* Component );

	/**
	 * Called when a component has been detached from an entity that exists in the scene.
	 */
	void onDetachComponent( ScnEntityWeakRef Entity, ScnComponent* Component );

private:
	void processPendingComponents();

private:
	ScnEntity* internalSpawnEntity( ScnEntitySpawnParams Params );
	void onSpawnEntityPackageReady( CsPackage* pPackage, BcU32 ID );

private:
	class ScnSpatialTree* pSpatialTree_;

	// Pending components.
	ScnComponentList PendingAttachComponentList_;
	ScnComponentList PendingDetachComponentList_;
	
	// Special components.
	ScnComponentList ViewComponentList_;

	// Callbacks.
	std::vector< class ScnCoreCallback* > Callbacks_;

	// All components in the scene.
	using ComponentClassIndexMap = std::unordered_map< const ReClass*, BcU32 >;
	using ComponentIndexClassMap = std::unordered_map< BcU32, const ReClass* >;
	using ComponentProcessorList = std::vector< ScnComponentProcessor >;
	using IndexedProcessFuncEntry = std::pair< BcU32, ScnComponentProcessFuncEntryList >;
	using IndexedProcessFuncEntryList = std::vector< ComponentProcessFuncEntryPair >;
	IndexedProcessFuncEntryList IndexedProcessFuncEntries_;
	ScnComponentList* pComponentLists_;
	BcU32 NoofComponentLists_;


	ComponentClassIndexMap ComponentClassIndexMap_;
	ComponentIndexClassMap ComponentIndexClassMap_;
	
	using EntitySpawnDataMap = std::map< BcU32, ScnEntitySpawnParams >;

	BcU32 EntitySpawnID_;
	EntitySpawnDataMap EntitySpawnMap_;

	// Debug components.
	ScnComponentList DebugComponents_;
};

template < class _Ty >
inline const ScnComponentList& ScnCore::getComponentList() const
{
	auto Class = _Ty::StaticGetClass();
	auto FoundIt = ComponentClassIndexMap_.find( Class );
	BcAssert( FoundIt != ComponentClassIndexMap_.end() );
	return pComponentLists_[ FoundIt.second ];
}

#endif


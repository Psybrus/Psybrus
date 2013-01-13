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
#include "System/Scene/ScnSpatialTree.h"

//////////////////////////////////////////////////////////////////////////
// Special scene components.
#include "System/Scene/ScnViewComponent.h"


//////////////////////////////////////////////////////////////////////////
// ScnCore
class ScnCore:
	public SysSystem,
	public BcGlobal< ScnCore >
{
public:
	ScnCore();
	virtual ~ScnCore();

	virtual void				open();
	virtual void				update();
	virtual void				close();

	/**
	 * Adds entity into the scene and ready to recieve updates.
	 */
	void						addEntity( ScnEntityRef Entity );

	/**
	 * Remove entity from the scene so it no longer recieves updates.
	 */
	void						removeEntity( ScnEntityRef Entity );

	/**
	 * Remove all entities from the scene.
	 */
	void						removeAllEntities();

	/**
	 * Create an entity from template.
	 */
	ScnEntityRef				createEntity( const BcName& Package, const BcName& Name, const BcName& InstanceName = BcName::INVALID );

	/**
	 * Spawn an entity from template. Handles loading and scene attachment.
	 */
	void						spawnEntity( const ScnEntitySpawnParams& Params );

	/**
	 * Find an entity. Non recursive, only searching within the manager, not parented entities.
	 */
	ScnEntityRef				findEntity( const BcName& InstanceName );

	/**
	 * Get an entity.
	 */
	ScnEntityRef				getEntity( BcU32 Idx );

	/**
	 * Queue component for attach/detach.
	 */
	void						queueComponentAsPendingOperation( ScnComponentRef Component );

	/**
	 * Visit view.
	 */
	void						visitView( class ScnVisitor* pVisitor, const RsViewport& Viewport );

	/**
	 * Visit bounds.
	 */
	void						visitBounds( class ScnVisitor* pVisitor, const BcAABB& Bounds );


private:
	/**
	 * Called when a component has been attached to an entity that exists in the scene.
	 */
	void						onAttachComponent( ScnEntityWeakRef Entity, ScnComponent* Component );

	/**
	 * Called when a component has been detached from an entity that exists in the scene.
	 */
	void						onDetachComponent( ScnEntityWeakRef Entity, ScnComponent* Component );

private:
	void						processPendingComponents();

private:
	void						onSpawnEntityPackageReady( CsPackage* pPackage, BcU32 ID );

private:
	ScnSpatialTree*				pSpatialTree_;

	// Pending components.
	ScnComponentList			PendingComponentList_;
	
	// Special components.
	ScnViewComponentList		ViewComponentList_;

	// All components in the scene.
	typedef std::map< const BcReflectionClass*, BcU32 > TComponentClassIndexMap;
	ScnComponentList*			pComponentLists_;
	BcU32						NoofComponentLists_;

	TComponentClassIndexMap		ComponentClassIndexMap_;

	typedef std::map< BcU32, ScnEntitySpawnParams > TEntitySpawnDataMap;
	typedef TEntitySpawnDataMap::iterator TEntitySpawnDataMapIterator;

	BcU32						EntitySpawnID_;
	TEntitySpawnDataMap			EntitySpawnMap_;
};

#endif


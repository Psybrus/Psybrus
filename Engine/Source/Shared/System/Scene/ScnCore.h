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
	ScnEntityRef				createEntity( const BcName& Package, const BcName& Name );

private:
	friend class ScnEntity;

	/**
	 * Called when a component has been attached to an entity that exists in the scene.
	 */
	void						onAttachComponent( ScnEntityWeakRef Entity, ScnComponentRef Component );

	/**
	 * Called when a component has been detached from an entity that exists in the scene.
	 */
	void						onDetachComponent( ScnEntityWeakRef Entity, ScnComponentRef Component );

private:
	void						processAddRemove();

private:
	ScnSpatialTree*				pSpacialTree_;
	ScnEntityList				EntityList_;

	ScnEntityList				AddEntityList_;
	ScnEntityList				RemoveEntityList_;
	
	// Special components.
	ScnViewComponentList		ViewComponentList_;

};

#endif


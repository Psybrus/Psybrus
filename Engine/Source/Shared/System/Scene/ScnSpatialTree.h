/**************************************************************************
*
* File:		ScnSpatialTree.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Spatial Tree
*		
*
*
* 
**************************************************************************/

#ifndef __SCNSPACIALTREE_H__
#define __SCNSPACIALTREE_H__

#include "Math/MaFrustum.h"
#include "Math/MaOctTree.h"

#include "System/Scene/ScnComponent.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef std::vector< class ScnSpatialComponent* > ScnSpatialComponentList;

//////////////////////////////////////////////////////////////////////////
/**	\class ScnSpatialTreeNode
*	\brief A Component in the tree which can contain ScnRenderableComponents
*
*	
*/
class ScnSpatialTreeNode:
	public MaOctTreeNode
{
public:
	ScnSpatialTreeNode();
	virtual ~ScnSpatialTreeNode();
	
	/**
	 * Add component.
	 */
	void addComponent( ScnSpatialComponent* Component );
	
	/**
	 * Remove component.
	 */
	void removeComponent( ScnSpatialComponent* Component );
	
	/**
	 * Reinsert component.
	 */
	void reinsertComponent( ScnSpatialComponent* Component );

	/**
	 * Gather with frustum.
	 * @param Frustum Frustum which all returned components will be intersecting with.
	 * @param OutComponents List to contain output components.
	 */
	void gather( const MaFrustum& Frustum, ScnComponentList& OutComponents );
	
	/**
	 * Gather with AABB.
	 * @param AABB Axis aligned bounding box which all returned components will be intersecting with.
	 * @param OutComponents List to contain output components.
	 */
	void gather( const MaAABB& AABB, ScnComponentList& OutComponents );

	/**
	 * Visit view.
	 */
	[[deprecated]] void visitView( class ScnVisitor* pVisitor, const class ScnViewComponent* View );

	/**
	 * Visit bounds.
	 */
	[[deprecated]] void visitBounds( class ScnVisitor* pVisitor, const MaAABB& Bounds );

private:
	ScnSpatialComponentList	Components_;

};

//////////////////////////////////////////////////////////////////////////
/**	\class ScnSpatialTree
*	\brief Tree to store Components.
*
*	
*/
class ScnSpatialTree:
	public BcOctTree
{
public:
	ScnSpatialTree();
	virtual ~ScnSpatialTree();
	
	/**
	 *	Add Component.
	 */
	void addComponent( class ScnSpatialComponent* Component );

	/**
	 *	Remove Component.
	 */
	void removeComponent( class ScnSpatialComponent* Component );

	/**
	 * Gather with frustum.
	 * @param Frustum Frustum which all returned components will be intersecting with.
	 * @param OutComponents List to contain output components.
	 */
	void gather( const MaFrustum& Frustum, ScnComponentList& OutComponents );
	
	/**
	 * Gather with AABB.
	 * @param AABB Axis aligned bounding box which all returned components will be intersecting with.
	 * @param OutComponents List to contain output components.
	 */
	void gather( const MaAABB& AABB, ScnComponentList& OutComponents );

	/**
	 *	Visit view.
	 */
	[[deprecated]] void visitView( class ScnVisitor* pVisitor, const class ScnViewComponent* View );
	
	/**
	 *	Visit bounds.
	 */
	[[deprecated]] void visitBounds( class ScnVisitor* pVisitor, const MaAABB& Bounds );



private:
	/**
	 *	Create our own Component type.
	 */
	virtual MaOctTreeNode*	createNode( const MaAABB& AABB );
};

#endif

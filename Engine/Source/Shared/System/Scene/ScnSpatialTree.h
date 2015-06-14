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

#include "Math/MaOctTree.h"

#include <list>

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
	 *	Add entity.
	 */
	void addComponent( ScnSpatialComponent* Component );
	
	/**
	 *	Remove entity.
	 */
	void removeComponent( ScnSpatialComponent* Component );
	
	/**
	 *	Reinsert entity.
	 */
	void reinsertComponent( ScnSpatialComponent* Component );
	
	/**
	 *	Visit view.
	 */
	[[deprecated]] void visitView( class ScnVisitor* pVisitor, const class ScnViewComponent* View );

	/**
	 *	Visit bounds.
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

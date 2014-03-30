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

#include "Base/BcOctTree.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef std::list< class ScnSpatialComponent* > ScnSpatialComponentList;

//////////////////////////////////////////////////////////////////////////
/**	\class ScnSpatialTreeNode
*	\brief A Component in the tree which can contain ScnRenderableComponents
*
*	
*/
class ScnSpatialTreeNode:
	public BcOctTreeNode
{
public:
	ScnSpatialTreeNode();
	virtual ~ScnSpatialTreeNode();
	
	/**
	*	Add entity.
	*/
	void					addComponent( ScnSpatialComponent* Component );
	
	/**
	*	Remove entity.
	*/
	void					removeComponent( ScnSpatialComponent* Component );
	
	/**
	*	Reinsert entity.
	*/
	void					reinsertComponent( ScnSpatialComponent* Component );
	
	/**
	*	Visit view.
	*/
	void					visitView( class ScnVisitor* pVisitor, const class ScnViewComponent* View );

	/**
	*	Visit bounds.
	*/
	void					visitBounds( class ScnVisitor* pVisitor, const BcAABB& Bounds );

private:
	ScnSpatialComponentList	ComponentList_;

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
	void					addComponent( class ScnSpatialComponent* Component );

	/**
	*	Remove Component.
	*/
	void					removeComponent( class ScnSpatialComponent* Component );

	/**
	*	Visit view.
	*/
	void					visitView( class ScnVisitor* pVisitor, const class ScnViewComponent* View );
	
	/**
	*	Visit bounds.
	*/
	void					visitBounds( class ScnVisitor* pVisitor, const BcAABB& Bounds );

private:
	/**
	*	Create our own Component type.
	*/
	virtual BcOctTreeNode*	createNode( const BcAABB& AABB );
};

#endif

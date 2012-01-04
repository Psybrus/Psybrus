/**************************************************************************
*
* File:		ScnSpatialTree.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Spacial Tree
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSPACIALTREE_H__
#define __ScnSPACIALTREE_H__

#include "BcOctTree.h"
#include "RsCore.h"
#include "ScnVisitor.h"
#include "ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
#define SCN_ENTITYLIST_DIVIDESIZE	8
#define SCN_MAX_RENDERNODES			1024
typedef std::list< ScnComponentWeakRef > ScnComponentWeakRefList;

//////////////////////////////////////////////////////////////////////////
/**	\class ScnSpatialTreeNode
*	\brief A Component in the tree which can contain ScnComponents
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
	void					addComponent( ScnComponentWeakRef Component );
	
	/**
	*	Remove entity.
	*/
	void					removeComponent( ScnComponentWeakRef Component );
	
	/**
	*	Reinsert entity.
	*/
	void					reinsertComponent( ScnComponentWeakRef Component );
	
	/**
	*	Visit view.
	*/
	void					visitView( ScnVisitor* pVisitor, const RsViewport& Viewport );

	/**
	*	Visit bounds.
	*/
	void					visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds );

private:
	ScnComponentWeakRefList	ComponentList_;

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
	void					addComponent( ScnComponentWeakRef Component );

	/**
	*	Remove Component.
	*/
	void					removeComponent( ScnComponentWeakRef Component );

	/**
	*	Visit view.
	*/
	void					visitView( ScnVisitor* pVisitor, const RsViewport& Viewport );
	
	/**
	*	Visit bounds.
	*/
	void					visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds );

private:
	/**
	*	Create our own Component type.
	*/
	virtual BcOctTreeNode*	createNode( const BcAABB& AABB );
};

#endif

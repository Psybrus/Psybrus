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

#ifndef __SCNSPACIALTREE_H__
#define __SCNSPACIALTREE_H__

#include "Base/BcOctTree.h"
#include "System/Renderer/RsCore.h"
#include "System/Scene/ScnVisitor.h"
#include "System/Scene/ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
#define SCN_ENTITYLIST_DIVIDESIZE	8
#define SCN_MAX_RENDERNODES			1024
typedef std::list< ScnRenderableComponentWeakRef > ScnRenderableComponentWeakRefList;

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
	void					addComponent( ScnRenderableComponentWeakRef Component );
	
	/**
	*	Remove entity.
	*/
	void					removeComponent( ScnRenderableComponentWeakRef Component );
	
	/**
	*	Reinsert entity.
	*/
	void					reinsertComponent( ScnRenderableComponentWeakRef Component );
	
	/**
	*	Visit view.
	*/
	void					visitView( ScnVisitor* pVisitor, const RsViewport& Viewport );

	/**
	*	Visit bounds.
	*/
	void					visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds );

private:
	ScnRenderableComponentWeakRefList	ComponentList_;

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
	void					addComponent( ScnRenderableComponentWeakRef Component );

	/**
	*	Remove Component.
	*/
	void					removeComponent( ScnRenderableComponentWeakRef Component );

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

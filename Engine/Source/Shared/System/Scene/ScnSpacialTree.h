/**************************************************************************
*
* File:		SgSpacialTree.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Spacial Tree
*		
*
*
* 
**************************************************************************/

#ifndef __SGSPACIALTREE_H__
#define __SGSPACIALTREE_H__

#include "BcOctTree.h"
#include "RsCore.h"
#include "SgVisitor.h"

#include <list>

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SgNode;

//////////////////////////////////////////////////////////////////////////
// Typedefs
#define SG_NODELIST_DIVIDESIZE	8
#define SG_MAX_RENDERNODES		1024
typedef std::list< SgNode* > SgNodeList;

//////////////////////////////////////////////////////////////////////////
/**	\class SgSpacialTreeNode
*	\brief A node in the tree which can contain SgNodes
*
*	
*/
class SgSpacialTreeNode:
	public BcOctTreeNode,
	public BcMemoryAllocator< bcMEM_SCENEGRAPH >
{
public:
	SgSpacialTreeNode();
	virtual ~SgSpacialTreeNode();
	
	/**
	*	Add node.
	*/
	void					addNode( SgNode* pNode );
	
	/**
	*	Remove node.
	*/
	void					removeNode( SgNode* pNode );
	
	/**
	*	Reinsert node.
	*/
	void					reinsertNode( SgNode* pNode );
	
	/**
	*	Visit view.
	*/
	void					visitView( SgVisitor* pVisitor, const RsViewport& Viewport );

	/**
	*	Visit bounds.
	*/
	void					visitBounds( SgVisitor* pVisitor, const BcAABB& Bounds );

private:
	SgNodeList				NodeList_;

};

//////////////////////////////////////////////////////////////////////////
/**	\class SgSpacialTree
*	\brief Tree to store nodes.
*
*	
*/
class SgSpacialTree:
	public BcOctTree
{
public:
	SgSpacialTree();
	virtual ~SgSpacialTree();
	
	/**
	*	Add node.
	*/
	void					addNode( SgNode* pNode );

	/**
	*	Remove node.
	*/
	void					removeNode( SgNode* pNode );

	/**
	*	Visit view.
	*/
	void					visitView( SgVisitor* pVisitor, const RsViewport& Viewport );
	
	/**
	*	Visit bounds.
	*/
	void					visitBounds( SgVisitor* pVisitor, const BcAABB& Bounds );

private:
	/**
	*	Create our own node type.
	*/
	virtual BcOctTreeNode*	createNode( const BcAABB& AABB );
};

#endif

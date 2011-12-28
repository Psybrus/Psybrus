/**************************************************************************
*
* File:		ScnSpacialTree.h
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
typedef std::list< ScnEntityWeakRef > ScnEntityWeakRefList;

//////////////////////////////////////////////////////////////////////////
/**	\class ScnSpacialTreeNode
*	\brief A Entity in the tree which can contain ScnEntitys
*
*	
*/
class ScnSpacialTreeNode:
	public BcOctTreeNode
{
public:
	ScnSpacialTreeNode();
	virtual ~ScnSpacialTreeNode();
	
	/**
	*	Add entity.
	*/
	void					addEntity( ScnEntityWeakRef Entity );
	
	/**
	*	Remove entity.
	*/
	void					removeEntity( ScnEntityWeakRef Entity );
	
	/**
	*	Reinsert entity.
	*/
	void					reinsertEntity( ScnEntityWeakRef Entity );
	
	/**
	*	Visit view.
	*/
	void					visitView( ScnVisitor* pVisitor, const RsViewport& Viewport );

	/**
	*	Visit bounds.
	*/
	void					visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds );

private:
	ScnEntityWeakRefList				EntityList_;

};

//////////////////////////////////////////////////////////////////////////
/**	\class ScnSpacialTree
*	\brief Tree to store Entitys.
*
*	
*/
class ScnSpacialTree:
	public BcOctTree
{
public:
	ScnSpacialTree();
	virtual ~ScnSpacialTree();
	
	/**
	*	Add Entity.
	*/
	void					addEntity( ScnEntityWeakRef Entity );

	/**
	*	Remove Entity.
	*/
	void					removeEntity( ScnEntityWeakRef Entity );

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
	*	Create our own Entity type.
	*/
	virtual BcOctTreeNode*	createNode( const BcAABB& AABB );
};

#endif

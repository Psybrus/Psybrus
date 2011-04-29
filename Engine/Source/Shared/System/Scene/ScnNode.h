/**************************************************************************
*
* File:		ScnNode.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scene Node
*			- process is called before update.		
*
*
* 
**************************************************************************/

#ifndef __SCNNODE_H__
#define __SCNNODE_H__

#include "ScnVisitor.h"

#include "BcAABB.h"
#include "BcMat4d.h"
#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class ScnSpacialTree;
class ScnSpacialTreeNode;

//////////////////////////////////////////////////////////////////////////
/**	\class ScnNode
*	\brief Scene Node
*
*	Hides away all that tedious rendering and updating.
*/
class ScnNode
{
public:
	DECLARE_VISITABLE( ScnNode );

public:
	ScnNode();
	virtual ~ScnNode();

	/**
	*	Add a node to this one.
	*/
	virtual void addNode( ScnNode* pNode );

	/**
	*	Remove a node from this one.
	*/
	virtual void removeNode( ScnNode* pNode );

	/**
	*	Update a node. Called by ScnNodeImpl.
	*/
	virtual void update();
	
	/**
	*	Set the local transform of this node.
	*/
	void localTransform( const BcMat4d& LocalTransform );

	/**
	*	Get the local transform of this node.
	*/
	const BcMat4d& localTransform() const;

	/**
	*	Set the world transform of this node.
	*/
	void worldTransform( const BcMat4d& WorldTransform );

	/**
	*	Get the world transform of this node.
	*/
	const BcMat4d& worldTransform() const;

	/**
	*	Set this node's AABB. Use with caution! Done in packer stage.
	*/
	void aabb( const BcAABB& aabb );

	/**
	*	Get this node's world space AABB.
	*/
	const BcAABB& aabb() const;

	/**
	*	Get this node's and its childrens combined world space AABB.
	*/
	BcAABB aabbRecursive() const;

	/**
	*	This node's parent.
	*/
	ScnNode* pParent();

	/**
	*	This node's first child.
	*/
	ScnNode* pChild();

	/**
	*	Parent's node's next child.
	*/
	ScnNode* pNext();

	/**
	*	Visit bottom up.
	*/
	void	visitBottomUp( ScnVisitor* pVisitor );
	
	/**
	*	Visit top down.
	*/
	void	visitTopDown( ScnVisitor* pVisitor );

protected:
	friend class ScnSpacialTree;
	friend class ScnSpacialTreeNode;
	
	//
	ScnSpacialTreeNode* pSpacialNode_;		// Spacial Node we belong to.
	
	//
	ScnNode*			pParent_;				// Parent
	ScnNode*			pChild_;				// Child
	ScnNode*			pNext_;					// Next Child of our Parent
	
	//
	BcMat4d				LocalTransform_;		// Local Space
	BcMat4d				WorldTransform_;		// World Space
	BcAABB				AABB_;					// AABB
	BcAABB				TransformedAABB_;		// Transformed AABB
	
	//	
	BcU32				nChildren_;
	
	//
	BcBool				IsAttached_		: 1;	// Has been attached by the user.
	
private:

};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline void ScnNode::localTransform( const BcMat4d& LocalTransform )
{
	LocalTransform_ = LocalTransform;
}

inline const BcMat4d& ScnNode::localTransform() const
{
	return LocalTransform_;
}

inline void ScnNode::worldTransform( const BcMat4d& WorldTransform )
{
	WorldTransform_ = WorldTransform;
}

inline const BcMat4d& ScnNode::worldTransform() const
{
	return WorldTransform_;
}

inline void ScnNode::aabb( const BcAABB& AABB )
{
	AABB_ = AABB;
}

inline const BcAABB& ScnNode::aabb() const
{
	return TransformedAABB_;
}

inline ScnNode* ScnNode::pParent()
{
	return pParent_;
}

inline ScnNode* ScnNode::pChild()
{
	return pChild_;
}

inline ScnNode* ScnNode::pNext()
{
	return pNext_;
}

#endif

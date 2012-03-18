/**************************************************************************
*
* File:		BcOctTree.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Octtree structure.
*		You can derive from BcOctTreeNode to have your own functionality
*		for each node, though you must derive from BcOctTree and overload
*		its creation function.
* 
**************************************************************************/

#ifndef __BCOCTTREE_H__
#define __BCOCTTREE_H__

#include "Base/BcAABB.h"
#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class BcOctTreeNode;
class BcOctTree;

//////////////////////////////////////////////////////////////////////////
// BcOctTreeNode
class BcOctTreeNode
{
public:
	BcOctTreeNode();
	virtual ~BcOctTreeNode();

	const BcAABB& getAABB() const;
	void setAABB( const BcAABB& AABB );

	BcOctTreeNode* pParent();
	BcOctTreeNode* pChild( BcU32 iChild );

	BcOctTree* pTree();

	void subDivide();

protected:

private:
	friend class BcOctTree;

	// Tree structure
	BcOctTree*						pTree_;
	BcOctTreeNode*					pParent_;
	BcOctTreeNode*					aChildNodes_[ 8 ];

	// Bounds
	BcAABB							AABB_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcOctTreeNode::BcOctTreeNode():
	pTree_( NULL ),
	pParent_( NULL )
{
	// Clear child nodes.
	BcMemSet( &aChildNodes_, 0, sizeof( aChildNodes_ ) );
}

inline BcOctTreeNode::~BcOctTreeNode()
{
	for( BcU32 i = 0; i < 8; ++i )
	{
		if( aChildNodes_[ i ] != NULL )
		{
			delete aChildNodes_[ i ];
		}
	}
}

inline const BcAABB& BcOctTreeNode::getAABB() const
{
	return AABB_;
}

inline void BcOctTreeNode::setAABB( const BcAABB& AABB )
{
	AABB_ = AABB;
}

inline BcOctTreeNode* BcOctTreeNode::pParent()
{
	return pParent_;
}

inline BcOctTreeNode* BcOctTreeNode::pChild( BcU32 iChild )
{
	return aChildNodes_[ iChild ];
}

inline BcOctTree* BcOctTreeNode::pTree()
{
	return pTree_;
}

//////////////////////////////////////////////////////////////////////////
// BcOctTree
class BcOctTree
{
public:
	BcOctTree();
	virtual ~BcOctTree();

	// Base
	void createRoot( const BcAABB& AABB );

	// Utility
	BcOctTreeNode* pRootNode();
	void subDivide( BcOctTreeNode* pNode );

	//
	BcOctTreeNode* findNode( const BcVec3d& Point );
	BcOctTreeNode* findNode( BcOctTreeNode* pNode, const BcVec3d& Point );

	BcOctTreeNode* findNode( const BcAABB& AABB );
	BcOctTreeNode* findNode( BcOctTreeNode* pNode, const BcAABB& AABB );

protected:
	// Overloads
	virtual BcOctTreeNode* createNode( const BcAABB& AABB );

private:
	BcOctTreeNode* pRootNode_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcOctTreeNode* BcOctTree::pRootNode()
{
	return pRootNode_;
}

#endif

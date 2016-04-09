/**************************************************************************
*
* File:		MaOctTree.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Octtree structure.
*		You can derive from MaOctTreeNode to have your own functionality
*		for each node, though you must derive from MaOctTree and overload
*		its creation function.
* 
**************************************************************************/

#ifndef __BCOCTTREE_H__
#define __BCOCTTREE_H__

#include "Math/MaAABB.h"
#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class MaOctTreeNode;
class MaOctTree;

//////////////////////////////////////////////////////////////////////////
// MaOctTreeNode
class MaOctTreeNode
{
public:
	MaOctTreeNode();
	virtual ~MaOctTreeNode();

	const MaAABB& getAABB() const;
	void setAABB( const MaAABB& AABB );

	MaOctTreeNode* pParent();
	MaOctTreeNode* pChild( BcU32 iChild );

	MaOctTree* pTree();

	void subDivide();

protected:

private:
	friend class MaOctTree;

	// Tree structure
	MaOctTree*						pTree_;
	MaOctTreeNode*					pParent_;
	MaOctTreeNode*					aChildNodes_[ 8 ];

	// Bounds
	MaAABB							AABB_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline MaOctTreeNode::MaOctTreeNode():
	pTree_( NULL ),
	pParent_( NULL )
{
	// Clear child nodes.
	BcMemSet( &aChildNodes_, 0, sizeof( aChildNodes_ ) );
}

inline MaOctTreeNode::~MaOctTreeNode()
{
	for( BcU32 i = 0; i < 8; ++i )
	{
		if( aChildNodes_[ i ] != NULL )
		{
			delete aChildNodes_[ i ];
		}
	}
}

inline const MaAABB& MaOctTreeNode::getAABB() const
{
	return AABB_;
}

inline void MaOctTreeNode::setAABB( const MaAABB& AABB )
{
	AABB_ = AABB;
}

inline MaOctTreeNode* MaOctTreeNode::pParent()
{
	return pParent_;
}

inline MaOctTreeNode* MaOctTreeNode::pChild( BcU32 iChild )
{
	return aChildNodes_[ iChild ];
}

inline MaOctTree* MaOctTreeNode::pTree()
{
	return pTree_;
}

//////////////////////////////////////////////////////////////////////////
// MaOctTree
class MaOctTree
{
public:
	MaOctTree();
	virtual ~MaOctTree();

	// Base
	void createRoot( const MaAABB& AABB );

	// Utility
	MaOctTreeNode* pRootNode();
	void subDivide( MaOctTreeNode* pNode );

	//
	MaOctTreeNode* findNode( const MaVec3d& Point );
	MaOctTreeNode* findNode( MaOctTreeNode* pNode, const MaVec3d& Point );

	MaOctTreeNode* findNode( const MaAABB& AABB );
	MaOctTreeNode* findNode( MaOctTreeNode* pNode, const MaAABB& AABB );

protected:
	// Overloads
	virtual MaOctTreeNode* createNode( const MaAABB& AABB );

private:
	MaOctTreeNode* pRootNode_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline MaOctTreeNode* MaOctTree::pRootNode()
{
	return pRootNode_;
}

#endif

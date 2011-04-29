/**************************************************************************
*
* File:		BcOctTree.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Octtree structure.
*		You can derive from BcOctTreeNode to have your own functionality
*		for each node, though you must derive from BcOctTree and overload
*		its creation function.
* 
**************************************************************************/

#include "BcOctTree.h"

//////////////////////////////////////////////////////////////////////////
// subDivide
void BcOctTreeNode::subDivide()
{
	pTree_->subDivide( this );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcOctTree::BcOctTree():
	pRootNode_( NULL )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcOctTree::~BcOctTree()
{
	if( pRootNode_ != NULL )
	{
		delete pRootNode_;
	}
}

//////////////////////////////////////////////////////////////////////////
// createRoot
void BcOctTree::createRoot( const BcAABB& AABB )
{
	if( pRootNode_ != NULL )
	{
		delete pRootNode_;
	}

	pRootNode_ = createNode( AABB );
	pRootNode_->pParent_ = NULL;
	pRootNode_->pTree_ = this;

}

//////////////////////////////////////////////////////////////////////////
// subDivide
void BcOctTree::subDivide( BcOctTreeNode* pNode )
{
	//
	BcVec3d Center = pNode->AABB_.centre();

	// Create the 8 corners and setup child nodes.
	for( BcU32 i = 0; i < 8; ++i )
	{
		// Create AABB for this corner.
		BcAABB NewAABB;

		NewAABB.expandBy( Center );
		NewAABB.expandBy( pNode->AABB_.corner( i ) );

		// Setup child node.
		pNode->aChildNodes_[ i ] = createNode( NewAABB );
		pNode->aChildNodes_[ i ]->pParent_ = pNode;
		pNode->aChildNodes_[ i ]->pTree_ = this;
	}
}

//////////////////////////////////////////////////////////////////////////
// findNode
BcOctTreeNode* BcOctTree::findNode( const BcVec3d& Point )
{
	return findNode( pRootNode_, Point );
}

//////////////////////////////////////////////////////////////////////////
// findNode
BcOctTreeNode* BcOctTree::findNode( BcOctTreeNode* pNode, const BcVec3d& Point )
{
	//
	BcOctTreeNode* pRetNode = NULL;

	// If the point is in the node, check children.
	if( pNode->aabb().classify( Point ) == BcAABB::bcBC_INSIDE )
	{
		// Check children.
		if( pNode->pChild( 0 ) != NULL )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				BcOctTreeNode* pChildNode;

				pChildNode = findNode( pNode->pChild( i ), Point );

				// One of our children is the bearer of the point.
				if( pChildNode != NULL )
				{
					pRetNode = pChildNode;
					break;
				}
			}
		}
		else
		{
			// We are the bearer of the point.
			pRetNode = pNode;
		}
	}

	return pRetNode;
}

//////////////////////////////////////////////////////////////////////////
// findNode
BcOctTreeNode* BcOctTree::findNode( const BcAABB& AABB )
{
	return findNode( pRootNode_, AABB );
}

//////////////////////////////////////////////////////////////////////////
// findNode
BcOctTreeNode* BcOctTree::findNode( BcOctTreeNode* pNode, const BcAABB& AABB )
{
	//
	BcOctTreeNode* pRetNode = NULL;

	BcBreakpoint;
	
	/*
	// If the point is in the node, check children.
	if( pNode->aabb().classify( AABB ) == BcAABB::bcBC_INSIDE )
	{
		// Check children.
		if( pNode->pChild( 0 ) != NULL )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				BcOctTreeNode* pChildNode;

				pChildNode = findNode( pNode->pChild( i ), AABB );

				// One of our children is the bearer of the point.
				if( pChildNode != NULL )
				{
					pRetNode = pChildNode;
					break;
				}
			}
		}
		else
		{
			// We are the bearer of the point.
			pRetNode = pNode;
		}
	}
	*/

	return pRetNode;
}

//////////////////////////////////////////////////////////////////////////
// Overloads
//virtual 
BcOctTreeNode* BcOctTree::createNode( const BcAABB& AABB )
{
	// Create node.
	BcOctTreeNode* pNode = new BcOctTreeNode();

	// Set AABB.
	pNode->AABB_ = AABB;

	return pNode;
}

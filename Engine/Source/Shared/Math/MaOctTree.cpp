/**************************************************************************
*
* File:		BcOctTree.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Octtree structure.
*		You can derive from MaOctTreeNode to have your own functionality
*		for each node, though you must derive from BcOctTree and overload
*		its creation function.
* 
**************************************************************************/

#include "Math/MaOctTree.h"

//////////////////////////////////////////////////////////////////////////
// subDivide
void MaOctTreeNode::subDivide()
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
void BcOctTree::createRoot( const MaAABB& AABB )
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
void BcOctTree::subDivide( MaOctTreeNode* pNode )
{
	//
	MaVec3d Center = pNode->AABB_.centre();

	// Create the 8 corners and setup child nodes.
	for( BcU32 i = 0; i < 8; ++i )
	{
		// Create AABB for this corner.
		MaAABB NewAABB;

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
MaOctTreeNode* BcOctTree::findNode( const MaVec3d& Point )
{
	return findNode( pRootNode_, Point );
}

//////////////////////////////////////////////////////////////////////////
// findNode
MaOctTreeNode* BcOctTree::findNode( MaOctTreeNode* pNode, const MaVec3d& Point )
{
	//
	MaOctTreeNode* pRetNode = NULL;

	// If the point is in the node, check children.
	if( pNode->getAABB().classify( Point ) == MaAABB::bcBC_INSIDE )
	{
		// Check children.
		if( pNode->pChild( 0 ) != NULL )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				MaOctTreeNode* pChildNode;

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
MaOctTreeNode* BcOctTree::findNode( const MaAABB& AABB )
{
	return findNode( pRootNode_, AABB );
}

//////////////////////////////////////////////////////////////////////////
// findNode
MaOctTreeNode* BcOctTree::findNode( MaOctTreeNode* pNode, const MaAABB& AABB )
{
	//
	MaOctTreeNode* pRetNode = NULL;

	BcBreakpoint;
	
	/*
	// If the point is in the node, check children.
	if( pNode->aabb().classify( AABB ) == MaAABB::bcBC_INSIDE )
	{
		// Check children.
		if( pNode->pChild( 0 ) != NULL )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				MaOctTreeNode* pChildNode;

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
MaOctTreeNode* BcOctTree::createNode( const MaAABB& AABB )
{
	// Create node.
	MaOctTreeNode* pNode = new MaOctTreeNode();

	// Set AABB.
	pNode->AABB_ = AABB;

	return pNode;
}

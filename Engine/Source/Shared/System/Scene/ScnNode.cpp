/**************************************************************************
*
* File:		ScnNode.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Node
*		
*
*
* 
**************************************************************************/

#include "ScnNode.h"
#include "ScnCore.h"
#include "ScnSpacialTree.h"

#include "RsCore.h"
#include "RsFrame.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnNode::ScnNode():
	pSpacialNode_( NULL ),
	pParent_( NULL ),
	pChild_( NULL ),
	pNext_( NULL ),
	pZone_( NULL ),
	nChildren_( 0 ),
	IsAttached_( BcFalse )
{
	LocalTransform_.identity();
	WorldTransform_.identity();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ScnNode::~ScnNode()
{

}

//////////////////////////////////////////////////////////////////////////
// addNode
void ScnNode::addNode( ScnNode* pNode )
{
	if( pNode->pParent_ != NULL )
	{
 		pNode->pParent_->removeNode( pNode );
		BcPrintf( "ScnNode: WARNING: Node has a parent.\n" );
	}

	ScnNode* pNextNode = pChild_;
	pChild_ = pNode;
 	pNode->pParent_ = this;
	pNode->pNext_ = pNextNode;
	pNode->IsAttached_ = BcTrue;
	++nChildren_;

	// TODO: Reimplement zones.
	//pNode->setZoneRecursive( pZone_ );
}

//////////////////////////////////////////////////////////////////////////
// removeNode
void ScnNode::removeNode( ScnNode* pNode )
{
	if( pNode->pParent_ == this )
	{
		ScnNode* pChild = pChild_;
		ScnNode* pPrevNode = NULL;
		ScnNode* pNextNode = NULL;

		while( pChild != NULL )
		{
			pNextNode = pChild->pNext_;

			if( pChild == pNode )
			{
				if( pPrevNode != NULL )
				{
					pPrevNode->pNext_ = pNextNode;
				}
				else
				{
					pChild_ = pNextNode;
					pChild = NULL;
				}
			}

			pPrevNode = pChild;
			pChild = pNextNode;
		}

		pNode->pParent_ = NULL;
		pNode->pNext_ = NULL;
		pNode->IsAttached_ = BcFalse;
		--nChildren_;

		// TODO: Reimplement zones.
		//pNode->setZoneRecursive( pZone_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
void ScnNode::update()
{
	//////////////////////////////////////////////////
	// Update transform
	if( pParent_ != NULL )
	{
		WorldTransform_ = LocalTransform_ * pParent_->WorldTransform_;
	}
	else
	{
		WorldTransform_ = LocalTransform_;
	}
	
	//////////////////////////////////////////////////
	// Update AABB
	if( !AABB_.isEmpty() )
	{
		TransformedAABB_ = AABB_.transform( WorldTransform_ );

		// Reinsert into spacial tree.
		if( pSpacialNode_ != NULL )
		{
			pSpacialNode_->reinsertNode( this );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// aabbRecursive
BcAABB ScnNode::aabbRecursive() const
{
	BcAABB AABB;
	
	// Our own.
	if( aabb().isEmpty() == BcFalse )
	{
		AABB.expandBy( aabb() );
	}

	// Children.
	ScnNode* pNextNode = pChild_;
	while( pNextNode != NULL )
	{
		BcAABB ChildAABB = pNextNode->aabbRecursive();
		if( ChildAABB.isEmpty() == BcFalse )
		{
			AABB.expandBy( ChildAABB );
		}
		pNextNode = pNextNode->pNext_;
	}

	//
	return AABB;
}

//////////////////////////////////////////////////////////////////////////
// visitBottomUp
void ScnNode::visitBottomUp( ScnVisitor* pVisitor )
{
	ScnNode* pIter = pChild();
	
	// Visit node.
	visit_accept( pVisitor );

	// Recurse children.
	while( pIter != NULL )
	{
		pIter->visitBottomUp( pVisitor );
		
		pIter = pIter->pNext();
	}
}

//////////////////////////////////////////////////////////////////////////
// visitTopDown
void ScnNode::visitTopDown( ScnVisitor* pVisitor )
{
	ScnNode* pIter = pChild();
	
	// Recurse children.
	while( pIter != NULL )
	{
		pIter->visitTopDown( pVisitor );
		
		pIter = pIter->pNext();
	}
	
	// Visit node.
	visit_accept( pVisitor );
}

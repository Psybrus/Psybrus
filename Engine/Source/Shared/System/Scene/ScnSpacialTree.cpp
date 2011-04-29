/**************************************************************************
*
* File:		SgSpacialTree.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Spacial Tree
*		
*
*
* 
**************************************************************************/

#include "SgSpacialTree.h"

#include "SgNode.h"

#include "RsCore.h"
#include "RsFrame.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SgSpacialTreeNode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
SgSpacialTreeNode::SgSpacialTreeNode()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SgSpacialTreeNode::~SgSpacialTreeNode()
{
	// Remove SgNodes from the list and clear their parent.
	SgNodeList::iterator Iter = NodeList_.begin();

	while( Iter != NodeList_.end() )
	{
		SgNode* pNode = *Iter;

		// Validate before removing.
		BcAssert( pNode->pSpacialNode_ == this );
		pNode->pSpacialNode_ = NULL;

		++Iter;
	}

	NodeList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// addNode
void SgSpacialTreeNode::addNode( SgNode* pNode )
{
	// Add to self.
	// TODO: Subdivide and such.
	NodeList_.push_back( pNode );
	pNode->pSpacialNode_ = this;

	// Reinsert node to put it into the correct child node.
	reinsertNode( pNode );
}

//////////////////////////////////////////////////////////////////////////
// removeNode
void SgSpacialTreeNode::removeNode( SgNode* pNode )
{
	// Remove from self/owner.
	if( pNode->pSpacialNode_ != this )
	{
		pNode->pSpacialNode_->removeNode( pNode );
	}

	NodeList_.remove( pNode );
	pNode->pSpacialNode_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// reinsertNode
void SgSpacialTreeNode::reinsertNode( SgNode* pNode )
{
	// If we've got no children, check if we need to subdivide.
	if( pChild( 0 ) == NULL )
	{
		if( NodeList_.size() > SG_NODELIST_DIVIDESIZE )
		{
			subDivide();
		}
	}

	// If node does not fit into self, give to parent, they can deal with it.
	if( pNode->aabb().isEmpty() == BcFalse && aabb().classify( pNode->aabb() ) != BcAABB::bcBC_INSIDE )
	{
		//BcAssertMsg( pParent() != NULL, "Object went outside of scene." );

		if( pParent() != NULL )
		{
			SgSpacialTreeNode* pParentNode = static_cast< SgSpacialTreeNode* >( pParent() );
			NodeList_.remove( pNode );
			pParentNode->NodeList_.push_back( pNode );
			pNode->pSpacialNode_ = pParentNode;
		}
	}
	else
	{
		// Give to children if we can.
		if( pChild( 0 ) != NULL )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				SgSpacialTreeNode* pChildNode = static_cast< SgSpacialTreeNode* >( pChild( i ) );

				BcAABB::eClassify Classification = pChildNode->aabb().classify( pNode->aabb() );

				// If its spanning, don't attempt any other children.
				if( Classification == BcAABB::bcBC_SPANNING )
				{
					break;
				}
				// If its inside the child, hand it over no questions asked.
				else if( Classification == BcAABB::bcBC_INSIDE )
				{
					NodeList_.remove( pNode );
					pChildNode->NodeList_.push_back( pNode );
					pNode->pSpacialNode_ = pChildNode;
				}
				// Otherwise we're safe to try another.
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitView
void SgSpacialTreeNode::visitView( SgVisitor* pVisitor, const RsViewport& Viewport )
{
	// Visit this nodes objects if they are inside the frustum.
	SgNodeList::iterator Iter = NodeList_.begin();

	while( Iter != NodeList_.end() )
	{
		SgNode* pNode = *Iter;

		if( pNode->aabb().isEmpty() || Viewport.intersect( pNode->aabb() ) == BcTrue )
		{
			pNode->visit_accept( pVisitor );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			SgSpacialTreeNode* pChildNode = static_cast< SgSpacialTreeNode* >( pChild( i ) );
			if( Viewport.intersect( pChildNode->aabb() ) )
			{
				pChildNode->visitView( pVisitor, Viewport );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void SgSpacialTreeNode::visitBounds( SgVisitor* pVisitor, const BcAABB& Bounds )
{
	// Visit this nodes objects if they are inside the frustum.
	SgNodeList::iterator Iter = NodeList_.begin();

	while( Iter != NodeList_.end() )
	{
		SgNode* pNode = *Iter;

		if( pNode->aabb().isEmpty() || Bounds.boxIntersect( pNode->aabb(), NULL ) == BcTrue )
		{
			pNode->visit_accept( pVisitor );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			SgSpacialTreeNode* pChildNode = static_cast< SgSpacialTreeNode* >( pChild( i ) );
			if( Bounds.boxIntersect( pChildNode->aabb(), NULL ) )
			{
				pChildNode->visitBounds( pVisitor, Bounds );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SgSpacialTree
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
SgSpacialTree::SgSpacialTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
SgSpacialTree::~SgSpacialTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// createNode
//virtual
BcOctTreeNode* SgSpacialTree::createNode( const BcAABB& AABB )
{
	SgSpacialTreeNode* pNode = new SgSpacialTreeNode();
	pNode->aabb( AABB );
	return pNode;
}

//////////////////////////////////////////////////////////////////////////
// addNode
void SgSpacialTree::addNode( SgNode* pNode )
{
	SgSpacialTreeNode* pRoot = static_cast< SgSpacialTreeNode* >( pRootNode() );

	pRoot->addNode( pNode );
}

//////////////////////////////////////////////////////////////////////////
// removeNode
void SgSpacialTree::removeNode( SgNode* pNode )
{
	SgSpacialTreeNode* pRoot = static_cast< SgSpacialTreeNode* >( pRootNode() );

	pRoot->removeNode( pNode );
}

//////////////////////////////////////////////////////////////////////////
// visitView
void SgSpacialTree::visitView( SgVisitor* pVisitor, const RsViewport& Viewport )
{
	SgSpacialTreeNode* pRoot = static_cast< SgSpacialTreeNode* >( pRootNode() );
	
	pRoot->visitView( pVisitor, Viewport );
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void SgSpacialTree::visitBounds( SgVisitor* pVisitor, const BcAABB& Bounds )
{
	SgSpacialTreeNode* pRoot = static_cast< SgSpacialTreeNode* >( pRootNode() );

	pRoot->visitBounds( pVisitor, Bounds );
}

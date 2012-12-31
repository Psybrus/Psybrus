/**************************************************************************
*
* File:		ScnSpatialTree.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Spacial Tree
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnSpatialTree.h"
#include "System/Scene/ScnRenderableComponent.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsFrame.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScnSpatialTreeNode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSpatialTreeNode::ScnSpatialTreeNode()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSpatialTreeNode::~ScnSpatialTreeNode()
{
	// Remove ScnRenderableComponents from the list and clear their parent.
	ScnRenderableComponentWeakRefList::iterator Iter = ComponentList_.begin();

	while( Iter != ComponentList_.end() )
	{
		ScnRenderableComponentWeakRef Component = *Iter;

		// Validate before removing.
		BcAssert( Component->getSpatialTreeNode() == this );
		Component->setSpatialTreeNode( NULL );

		++Iter;
	}

	ComponentList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// addComponent
void ScnSpatialTreeNode::addComponent( ScnRenderableComponentWeakRef Component )
{
	// Add to self.
	// TODO: Subdivide and such.
	ComponentList_.push_back( Component );
	Component->setSpatialTreeNode( this );

	// Reinsert Component to put it into the correct child Component.
	reinsertComponent( Component );
}

//////////////////////////////////////////////////////////////////////////
// removeComponent
void ScnSpatialTreeNode::removeComponent( ScnRenderableComponentWeakRef Component )
{
	// Remove from self/owner.
	ScnSpatialTreeNode* pNode = Component->getSpatialTreeNode();
	if( pNode != NULL && pNode != this )
	{
		pNode->removeComponent( Component );
	}

	ComponentList_.remove( Component );
	Component->setSpatialTreeNode( NULL );
}

//////////////////////////////////////////////////////////////////////////
// reinsertComponent
void ScnSpatialTreeNode::reinsertComponent( ScnRenderableComponentWeakRef Component )
{
	// If we've got no children, check if we need to subdivide.
	if( pChild( 0 ) == NULL )
	{
		if( ComponentList_.size() > SCN_ENTITYLIST_DIVIDESIZE )
		{
			subDivide();
		}
	}

	// If Component does not fit into self, give to parent, they can deal with it.
	if( Component->getAABB().isEmpty() == BcFalse && getAABB().classify( Component->getAABB() ) != BcAABB::bcBC_INSIDE )
	{
		//BcAssertMScn( pParent() != NULL, "Object went outside of scene." );

		if( pParent() != NULL )
		{
			ScnSpatialTreeNode* pParentNode = static_cast< ScnSpatialTreeNode* >( pParent() );
			ComponentList_.remove( Component );
			pParentNode->ComponentList_.push_back( Component );
			Component->setSpatialTreeNode( pParentNode );
		}
	}
	else
	{
		// Give to children if we can.
		if( pChild( 0 ) != NULL )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				ScnSpatialTreeNode* pChildNode = static_cast< ScnSpatialTreeNode* >( pChild( i ) );

				BcAABB::eClassify Classification = pChildNode->getAABB().classify( Component->getAABB() );

				// If its spanning, don't attempt any other children.
				if( Classification == BcAABB::bcBC_SPANNING )
				{
					break;
				}
				// If its inside the child, hand it over no questions asked.
				else if( Classification == BcAABB::bcBC_INSIDE )
				{
					ComponentList_.remove( Component );
					pChildNode->ComponentList_.push_back( Component );
					Component->setSpatialTreeNode( pChildNode );
					break;
				}
				// Otherwise we're safe to try another.
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnSpatialTreeNode::visitView( ScnVisitor* pVisitor, const RsViewport& Viewport )
{
	// Visit this Components objects if they are inside the frustum.
	ScnRenderableComponentWeakRefList::iterator Iter = ComponentList_.begin();

	while( Iter != ComponentList_.end() )
	{
		ScnRenderableComponentWeakRef Component = *Iter;

		if( Component->getAABB().isEmpty() || Viewport.intersect( Component->getAABB() ) == BcTrue )
		{
			Component->visit_accept( pVisitor );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			ScnSpatialTreeNode* pChildComponent = static_cast< ScnSpatialTreeNode* >( pChild( i ) );
			if( Viewport.intersect( pChildComponent->getAABB() ) )
			{
				pChildComponent->visitView( pVisitor, Viewport );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnSpatialTreeNode::visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds )
{
	// Visit this Components objects if they are inside the frustum.
	ScnRenderableComponentWeakRefList::iterator Iter = ComponentList_.begin();

	while( Iter != ComponentList_.end() )
	{
		ScnRenderableComponentWeakRef Component = *Iter;

		if( Component->getAABB().isEmpty() || Bounds.boxIntersect( Component->getAABB(), NULL ) == BcTrue )
		{
			Component->visit_accept( pVisitor );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			ScnSpatialTreeNode* pChildComponent = static_cast< ScnSpatialTreeNode* >( pChild( i ) );
			if( Bounds.boxIntersect( pChildComponent->getAABB(), NULL ) )
			{
				pChildComponent->visitBounds( pVisitor, Bounds );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScnSpatialTree
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSpatialTree::ScnSpatialTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSpatialTree::~ScnSpatialTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// createNode
//virtual
BcOctTreeNode* ScnSpatialTree::createNode( const BcAABB& AABB )
{
	ScnSpatialTreeNode* Component = new ScnSpatialTreeNode();
	Component->setAABB( AABB );
	return Component;
}

//////////////////////////////////////////////////////////////////////////
// addComponent
void ScnSpatialTree::addComponent( ScnRenderableComponentWeakRef Component )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->addComponent( Component );
}

//////////////////////////////////////////////////////////////////////////
// removeComponent
void ScnSpatialTree::removeComponent( ScnRenderableComponentWeakRef Component )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->removeComponent( Component );
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnSpatialTree::visitView( ScnVisitor* pVisitor, const RsViewport& Viewport )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );
	
	pRoot->visitView( pVisitor, Viewport );
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnSpatialTree::visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->visitBounds( pVisitor, Bounds );
}

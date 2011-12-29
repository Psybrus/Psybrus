/**************************************************************************
*
* File:		ScnSpacialTree.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Spacial Tree
*		
*
*
* 
**************************************************************************/

#include "ScnSpacialTree.h"
#include "ScnEntity.h"

#include "RsCore.h"
#include "RsFrame.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScnSpacialTreeNode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSpacialTreeNode::ScnSpacialTreeNode()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSpacialTreeNode::~ScnSpacialTreeNode()
{
	// Remove ScnEntitys from the list and clear their parent.
	ScnEntityWeakRefList::iterator Iter = EntityList_.begin();

	while( Iter != EntityList_.end() )
	{
		ScnEntityWeakRef Entity = *Iter;

		// Validate before removing.
		BcAssert( Entity->getSpacialTreeNode() == this );
		Entity->setSpacialTreeNode( NULL );

		++Iter;
	}

	EntityList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// addEntity
void ScnSpacialTreeNode::addEntity( ScnEntityWeakRef Entity )
{
	// Add to self.
	// TODO: Subdivide and such.
	EntityList_.push_back( Entity );
	Entity->setSpacialTreeNode( this );

	// Reinsert Entity to put it into the correct child Entity.
	reinsertEntity( Entity );
}

//////////////////////////////////////////////////////////////////////////
// removeEntity
void ScnSpacialTreeNode::removeEntity( ScnEntityWeakRef Entity )
{
	// Remove from self/owner.
	if( Entity->getSpacialTreeNode() != this )
	{
		Entity->getSpacialTreeNode()->removeEntity( Entity );
	}

	EntityList_.remove( Entity );
	Entity->setSpacialTreeNode( NULL );
}

//////////////////////////////////////////////////////////////////////////
// reinsertEntity
void ScnSpacialTreeNode::reinsertEntity( ScnEntityWeakRef Entity )
{
	// If we've got no children, check if we need to subdivide.
	if( pChild( 0 ) == NULL )
	{
		if( EntityList_.size() > SCN_ENTITYLIST_DIVIDESIZE )
		{
			subDivide();
		}
	}

	// If Entity does not fit into self, give to parent, they can deal with it.
	if( Entity->getAABB().isEmpty() == BcFalse && getAABB().classify( Entity->getAABB() ) != BcAABB::bcBC_INSIDE )
	{
		//BcAssertMScn( pParent() != NULL, "Object went outside of scene." );

		if( pParent() != NULL )
		{
			ScnSpacialTreeNode* pParentNode = static_cast< ScnSpacialTreeNode* >( pParent() );
			EntityList_.remove( Entity );
			pParentNode->EntityList_.push_back( Entity );
			Entity->setSpacialTreeNode( pParentNode );
		}
	}
	else
	{
		// Give to children if we can.
		if( pChild( 0 ) != NULL )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				ScnSpacialTreeNode* pChildNode = static_cast< ScnSpacialTreeNode* >( pChild( i ) );

				BcAABB::eClassify Classification = pChildNode->getAABB().classify( Entity->getAABB() );

				// If its spanning, don't attempt any other children.
				if( Classification == BcAABB::bcBC_SPANNING )
				{
					break;
				}
				// If its inside the child, hand it over no questions asked.
				else if( Classification == BcAABB::bcBC_INSIDE )
				{
					EntityList_.remove( Entity );
					pChildNode->EntityList_.push_back( Entity );
					Entity->setSpacialTreeNode( pChildNode );

					// NEILO NOTE: Shouldn't this, erm, break here?
				}
				// Otherwise we're safe to try another.
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnSpacialTreeNode::visitView( ScnVisitor* pVisitor, const RsViewport& Viewport )
{
	// Visit this Entitys objects if they are inside the frustum.
	ScnEntityWeakRefList::iterator Iter = EntityList_.begin();

	while( Iter != EntityList_.end() )
	{
		ScnEntityWeakRef Entity = *Iter;

		if( Entity->getAABB().isEmpty() || Viewport.intersect( Entity->getAABB() ) == BcTrue )
		{
			Entity->visit_accept( pVisitor );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			ScnSpacialTreeNode* pChildEntity = static_cast< ScnSpacialTreeNode* >( pChild( i ) );
			if( Viewport.intersect( pChildEntity->getAABB() ) )
			{
				pChildEntity->visitView( pVisitor, Viewport );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnSpacialTreeNode::visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds )
{
	// Visit this Entitys objects if they are inside the frustum.
	ScnEntityWeakRefList::iterator Iter = EntityList_.begin();

	while( Iter != EntityList_.end() )
	{
		ScnEntityWeakRef Entity = *Iter;

		if( Entity->getAABB().isEmpty() || Bounds.boxIntersect( Entity->getAABB(), NULL ) == BcTrue )
		{
			Entity->visit_accept( pVisitor );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			ScnSpacialTreeNode* pChildEntity = static_cast< ScnSpacialTreeNode* >( pChild( i ) );
			if( Bounds.boxIntersect( pChildEntity->getAABB(), NULL ) )
			{
				pChildEntity->visitBounds( pVisitor, Bounds );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScnSpacialTree
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSpacialTree::ScnSpacialTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSpacialTree::~ScnSpacialTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// createNode
//virtual
BcOctTreeNode* ScnSpacialTree::createNode( const BcAABB& AABB )
{
	ScnSpacialTreeNode* Entity = new ScnSpacialTreeNode();
	Entity->setAABB( AABB );
	return Entity;
}

//////////////////////////////////////////////////////////////////////////
// addEntity
void ScnSpacialTree::addEntity( ScnEntityWeakRef Entity )
{
	ScnSpacialTreeNode* pRoot = static_cast< ScnSpacialTreeNode* >( pRootNode() );

	pRoot->addEntity( Entity );
}

//////////////////////////////////////////////////////////////////////////
// removeEntity
void ScnSpacialTree::removeEntity( ScnEntityWeakRef Entity )
{
	ScnSpacialTreeNode* pRoot = static_cast< ScnSpacialTreeNode* >( pRootNode() );

	pRoot->removeEntity( Entity );
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnSpacialTree::visitView( ScnVisitor* pVisitor, const RsViewport& Viewport )
{
	ScnSpacialTreeNode* pRoot = static_cast< ScnSpacialTreeNode* >( pRootNode() );
	
	pRoot->visitView( pVisitor, Viewport );
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnSpacialTree::visitBounds( ScnVisitor* pVisitor, const BcAABB& Bounds )
{
	ScnSpacialTreeNode* pRoot = static_cast< ScnSpacialTreeNode* >( pRootNode() );

	pRoot->visitBounds( pVisitor, Bounds );
}

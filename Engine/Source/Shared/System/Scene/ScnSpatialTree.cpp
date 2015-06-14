/**************************************************************************
*
* File:		ScnSpatialTree.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Scenegraph Spatial Tree
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnSpatialTree.h"
#include "System/Scene/ScnTypes.h"
#include "System/Scene/ScnVisitor.h"
#include "System/Scene/ScnSpatialComponent.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsFrame.h"

#include "Base/BcProfiler.h"

#define SCN_ENTITYLIST_DIVIDESIZE	8

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
	ScnSpatialComponentList::iterator Iter = Components_.begin();

	while( Iter != Components_.end() )
	{
		ScnSpatialComponent* Component = *Iter;

		// Validate before removing.
		BcAssert( Component->getSpatialTreeNode() == this );
		Component->setSpatialTreeNode( NULL );

		++Iter;
	}

	Components_.clear();
}

//////////////////////////////////////////////////////////////////////////
// addComponent
void ScnSpatialTreeNode::addComponent( ScnSpatialComponent* Component )
{
	PSY_PROFILE_FUNCTION;
	// Add to self.
	// TODO: Subdivide and such.
	Components_.push_back( Component );
	Component->setSpatialTreeNode( this );

	// Reinsert Component to put it into the correct child Component.
	reinsertComponent( Component );
}

//////////////////////////////////////////////////////////////////////////
// removeComponent
void ScnSpatialTreeNode::removeComponent( ScnSpatialComponent* Component )
{
	PSY_PROFILE_FUNCTION;
	// Remove from self/owner.
	ScnSpatialTreeNode* pNode = Component->getSpatialTreeNode();
	if( pNode != NULL && pNode != this )
	{
		pNode->removeComponent( Component );
	}

	Components_.erase( std::find( Components_.begin(), Components_.end(), Component ) );
	Component->setSpatialTreeNode( NULL );
}

//////////////////////////////////////////////////////////////////////////
// reinsertComponent
void ScnSpatialTreeNode::reinsertComponent( ScnSpatialComponent* Component )
{
	PSY_PROFILE_FUNCTION;
	// If we've got no children, check if we need to subdivide.
	if( pChild( 0 ) == NULL )
	{
		if( Components_.size() > SCN_ENTITYLIST_DIVIDESIZE )
		{
			subDivide();
		}
	}

	// If Component does not fit into self, give to parent, they can deal with it.
	if( Component->getAABB().isEmpty() == BcFalse && getAABB().classify( Component->getAABB() ) != MaAABB::bcBC_INSIDE )
	{
		//BcAssertMScn( pParent() != NULL, "Object went outside of scene." );

		if( pParent() != NULL )
		{
			ScnSpatialTreeNode* pParentNode = static_cast< ScnSpatialTreeNode* >( pParent() );
			Components_.erase( std::find( Components_.begin(), Components_.end(), Component ) );
			pParentNode->Components_.push_back( Component );
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

				MaAABB::eClassify Classification = pChildNode->getAABB().classify( Component->getAABB() );

				// If its spanning, don't attempt any other children.
				if( Classification == MaAABB::bcBC_SPANNING )
				{
					break;
				}
				// If its inside the child, hand it over no questions asked.
				else if( Classification == MaAABB::bcBC_INSIDE )
				{
					Components_.erase( std::find( Components_.begin(), Components_.end(), Component ) );
					pChildNode->Components_.push_back( Component );
					Component->setSpatialTreeNode( pChildNode );
					break;
				}
				// Otherwise we're safe to try another.
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// gather
void ScnSpatialTreeNode::gather( const MaFrustum& Frustum, ScnComponentList& OutComponents )
{
	PSY_PROFILE_FUNCTION;

	// Visit this Components objects if they are inside the frustum.
	ScnSpatialComponentList::iterator Iter = Components_.begin();

	while( Iter != Components_.end() )
	{
		ScnSpatialComponent* Component = *Iter;

		const auto& AABB = Component->getAABB();
		const auto Radius = ( AABB.max() - AABB.min() ).magnitude() * 0.5f;

		if( Component->getAABB().isEmpty() || Frustum.intersect( AABB.centre(), Radius ) == BcTrue )
		{
			OutComponents.push_back( Component );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			ScnSpatialTreeNode* ChildNode = static_cast< ScnSpatialTreeNode* >( pChild( i ) );

			const auto& AABB = ChildNode->getAABB();
			const auto Radius = ( AABB.max() - AABB.min() ).magnitude() * 0.5f;

			if( Frustum.intersect( AABB.centre(), Radius ) )
			{
				ChildNode->gather( Frustum, OutComponents );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// gather
void ScnSpatialTreeNode::gather( const MaAABB& AABB, ScnComponentList& OutComponents )
{
	PSY_PROFILE_FUNCTION;
	// Visit this Components objects if they are inside the frustum.
	ScnSpatialComponentList::iterator Iter = Components_.begin();

	while( Iter != Components_.end() )
	{
		ScnSpatialComponent* Component = *Iter;

		if( Component->getAABB().isEmpty() || AABB.intersect( Component->getAABB(), NULL ) == BcTrue )
		{
			OutComponents.push_back( Component );
		}

		++Iter;
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			ScnSpatialTreeNode* ChildNode = static_cast< ScnSpatialTreeNode* >( pChild( i ) );
			if( AABB.intersect( ChildNode->getAABB(), NULL ) )
			{
				ChildNode->gather( AABB, OutComponents );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnSpatialTreeNode::visitView( ScnVisitor* pVisitor, const class ScnViewComponent* View )
{
	PSY_PROFILE_FUNCTION;
	// Visit this Components objects if they are inside the frustum.
	ScnSpatialComponentList::iterator Iter = Components_.begin();

	const auto& Frustum = View->getFrustum();

	while( Iter != Components_.end() )
	{
		ScnSpatialComponent* Component = *Iter;

		const auto& AABB = Component->getAABB();
		const auto Radius = ( AABB.max() - AABB.min() ).magnitude() * 0.5f;

		if( Component->getAABB().isEmpty() || Frustum.intersect( AABB.centre(), Radius ) == BcTrue )
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

			const auto& AABB = pChildComponent->getAABB();
			const auto Radius = ( AABB.max() - AABB.min() ).magnitude() * 0.5f;

			if( Frustum.intersect( AABB.centre(), Radius ) )
			{
				pChildComponent->visitView( pVisitor, View );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnSpatialTreeNode::visitBounds( ScnVisitor* pVisitor, const MaAABB& Bounds )
{
	PSY_PROFILE_FUNCTION;
	// Visit this Components objects if they are inside the frustum.
	ScnSpatialComponentList::iterator Iter = Components_.begin();

	while( Iter != Components_.end() )
	{
		ScnSpatialComponent* Component = *Iter;

		if( Component->getAABB().isEmpty() || Bounds.intersect( Component->getAABB(), NULL ) == BcTrue )
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
			if( Bounds.intersect( pChildComponent->getAABB(), NULL ) )
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
MaOctTreeNode* ScnSpatialTree::createNode( const MaAABB& AABB )
{
	ScnSpatialTreeNode* Component = new ScnSpatialTreeNode();
	Component->setAABB( AABB );
	return Component;
}

//////////////////////////////////////////////////////////////////////////
// addComponent
void ScnSpatialTree::addComponent( ScnSpatialComponent* Component )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->addComponent( Component );
}

//////////////////////////////////////////////////////////////////////////
// removeComponent
void ScnSpatialTree::removeComponent( ScnSpatialComponent* Component )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->removeComponent( Component );
}

//////////////////////////////////////////////////////////////////////////
// gather
void ScnSpatialTree::gather( const MaFrustum& Frustum, ScnComponentList& OutComponents )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->gather( Frustum, OutComponents );
}

//////////////////////////////////////////////////////////////////////////
// gather
void ScnSpatialTree::gather( const MaAABB& AABB, ScnComponentList& OutComponents )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->gather( AABB, OutComponents );
}

//////////////////////////////////////////////////////////////////////////
// visitView
void ScnSpatialTree::visitView( ScnVisitor* pVisitor, const ScnViewComponent* View )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );
	
	pRoot->visitView( pVisitor, View );
}

//////////////////////////////////////////////////////////////////////////
// visitBounds
void ScnSpatialTree::visitBounds( ScnVisitor* pVisitor, const MaAABB& Bounds )
{
	ScnSpatialTreeNode* pRoot = static_cast< ScnSpatialTreeNode* >( pRootNode() );

	pRoot->visitBounds( pVisitor, Bounds );
}

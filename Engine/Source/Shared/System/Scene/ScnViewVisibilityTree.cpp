#include "System/Scene/ScnViewVisibilityTree.h"
#include "System/Scene/ScnTypes.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderInterface.h"

#include "System/Renderer/RsCore.h"
#include "System/Renderer/RsFrame.h"

#include "Base/BcProfiler.h"

static const BcU32 GDivideSize( 32 );

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScnViewVisibilityTreeNode
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnViewVisibilityTreeNode::ScnViewVisibilityTreeNode()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnViewVisibilityTreeNode::~ScnViewVisibilityTreeNode()
{
	LeafList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// addLeaf
void ScnViewVisibilityTreeNode::addLeaf( ScnViewVisibilityLeaf* Leaf )
{
	PSY_PROFILE_FUNCTION;

	// Add to self.
	LeafList_.push_back( Leaf );
	Leaf->Node_ = this;

	// Reinsert Component to put it into the correct child Component.
	reinsertLeaf( Leaf );
}

//////////////////////////////////////////////////////////////////////////
// reinsertLeaf
void ScnViewVisibilityTreeNode::reinsertLeaf( ScnViewVisibilityLeaf* Leaf )
{
	PSY_PROFILE_FUNCTION;
	// If we've got no children, check if we need to subdivide.
	if( pChild( 0 ) == NULL )
	{
		if( LeafList_.size() > GDivideSize )
		{
			subDivide();
		}
	}

	// If Component does not fit into self, give to parent, they can deal with it.
	if( Leaf->AABB_.isEmpty() == BcFalse && 
		getAABB().classify( Leaf->AABB_ ) != MaAABB::bcBC_INSIDE )
	{
		if( pParent() != NULL )
		{
			ScnViewVisibilityTreeNode* ParentNode = static_cast< ScnViewVisibilityTreeNode* >( pParent() );
			auto It = std::find( LeafList_.begin(), LeafList_.end(), Leaf );
			if( It != LeafList_.end() )
			{
				LeafList_.erase( It );
			}
			ParentNode->LeafList_.push_back( Leaf );
			Leaf->Node_ = ParentNode;
		}
	}
	else
	{
		// Give to children if we can.
		if( pChild( 0 ) != nullptr )
		{
			for( BcU32 i = 0; i < 8; ++i )
			{
				ScnViewVisibilityTreeNode* ChildNode = static_cast< ScnViewVisibilityTreeNode* >( pChild( i ) );
				const MaAABB ChildAABB = ChildNode->getAABB();
				MaAABB::eClassify Classification = ChildAABB.classify( Leaf->AABB_ );

				// If its spanning, don't attempt any other children.
				if( Classification == MaAABB::bcBC_SPANNING )
				{
					break;
				}
				// If its inside the child, hand it over no questions asked.
				else if( Classification == MaAABB::bcBC_INSIDE )
				{
					auto It = std::find( LeafList_.begin(), LeafList_.end(), Leaf );
					if( It != LeafList_.end() )
					{
						LeafList_.erase( It );
					}
					ChildNode->LeafList_.push_back( Leaf );
					Leaf->Node_ = ChildNode;
					break;
				}
				// Otherwise we're safe to try another.
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// removeLeaf
void ScnViewVisibilityTreeNode::removeLeaf( ScnViewVisibilityLeaf* Leaf )
{
	PSY_PROFILE_FUNCTION;

	// Remove from self/owner.
	ScnViewVisibilityTreeNode* Node = Leaf->Node_;
	if( Node != nullptr && Node != this )
	{
		Node->removeLeaf( Leaf );
	}

	auto It = std::find( LeafList_.begin(), LeafList_.end(), Leaf );
	if( It != LeafList_.end() )
	{
		LeafList_.erase( It );
	}
	Leaf->Node_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// gatherView
void ScnViewVisibilityTreeNode::gatherView( const class ScnViewComponent* View, std::vector< ScnViewVisibilityLeaf* >& OutLeaves )
{
	PSY_PROFILE_FUNCTION;

	// Add components to output list.
	for( auto* Leaf : LeafList_ )
	{
		if( View == nullptr ||
			Leaf->AABB_.isEmpty() ||
			View->intersect( Leaf->AABB_ ) )
		{
			OutLeaves.push_back( Leaf );
		}
	}

	// Visit children if they are inside of the frustum.
	if( pChild( 0 ) != NULL )
	{
		for( BcU32 i = 0; i < 8; ++i )
		{
			ScnViewVisibilityTreeNode* ChildNode = static_cast< ScnViewVisibilityTreeNode* >( pChild( i ) );
			if( View == nullptr || View->intersect( ChildNode->getAABB() ) )
			{
				ChildNode->gatherView( View, OutLeaves );
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScnViewVisibilityTree
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnViewVisibilityTree::ScnViewVisibilityTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnViewVisibilityTree::~ScnViewVisibilityTree()
{
	
}

//////////////////////////////////////////////////////////////////////////
// createNode
//virtual
MaOctTreeNode* ScnViewVisibilityTree::createNode( const MaAABB& AABB )
{
	ScnViewVisibilityTreeNode* Component = new ScnViewVisibilityTreeNode();
	Component->setAABB( AABB );
	return Component;
}

//////////////////////////////////////////////////////////////////////////
// addLeaf
void ScnViewVisibilityTree::addLeaf( ScnViewVisibilityLeaf* Leaf )
{
	ScnViewVisibilityTreeNode* pRoot = static_cast< ScnViewVisibilityTreeNode* >( pRootNode() );

	pRoot->addLeaf( Leaf );
}

//////////////////////////////////////////////////////////////////////////
// removeComponent
void ScnViewVisibilityTree::removeLeaf( ScnViewVisibilityLeaf* Leaf )
{
	ScnViewVisibilityTreeNode* pRoot = static_cast< ScnViewVisibilityTreeNode* >( pRootNode() );

	pRoot->removeLeaf( Leaf );
}

//////////////////////////////////////////////////////////////////////////
// gatherView
void ScnViewVisibilityTree::gatherView( const ScnViewComponent* View, std::vector< ScnViewVisibilityLeaf* >& OutLeaves )
{
	ScnViewVisibilityTreeNode* pRoot = static_cast< ScnViewVisibilityTreeNode* >( pRootNode() );
	
	pRoot->gatherView( View, OutLeaves );
}

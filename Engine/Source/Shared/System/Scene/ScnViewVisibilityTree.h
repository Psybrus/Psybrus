#pragma once

#include "Math/MaOctTree.h"

#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// ScnViewVisibilityLeaf
struct ScnViewVisibilityLeaf
{
	class ScnViewVisibilityTreeNode* Node_ = nullptr;
	class ScnComponent* Component_ = nullptr;
	class ScnViewRenderInterface* RenderInterface_ = nullptr;
	BcU32 RenderMask_ = 0;
	MaAABB AABB_;
};

//////////////////////////////////////////////////////////////////////////
// ScnViewVisibilityTreeNode
class ScnViewVisibilityTreeNode:
	public MaOctTreeNode
{
public:
	ScnViewVisibilityTreeNode();
	virtual ~ScnViewVisibilityTreeNode();
	
	void addLeaf( ScnViewVisibilityLeaf* Leaf );
	void reinsertLeaf( ScnViewVisibilityLeaf* Leaf );
	void removeLeaf( ScnViewVisibilityLeaf* Leaf );
	
	/**
	 * Gather view.
	 */
	void gatherView( const class ScnViewComponent* View, std::vector< ScnViewVisibilityLeaf* >& OutLeaves );

private:
	std::vector< ScnViewVisibilityLeaf* > LeafList_;

};

//////////////////////////////////////////////////////////////////////////
// ScnViewVisibilityTree
class ScnViewVisibilityTree:
	public MaOctTree
{
public:
	ScnViewVisibilityTree();
	virtual ~ScnViewVisibilityTree();
	
	void addLeaf( ScnViewVisibilityLeaf* Leaf );
	void removeLeaf( ScnViewVisibilityLeaf* Leaf );
	void gatherView( const class ScnViewComponent* View, std::vector< ScnViewVisibilityLeaf* >& OutComponents );
	
private:
	virtual MaOctTreeNode* createNode( const MaAABB& AABB );
};


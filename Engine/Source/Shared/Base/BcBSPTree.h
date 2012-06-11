/**************************************************************************
*
* File:		BcBSPTree.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		BSP Tree implementation.
*		
*
*
* 
**************************************************************************/

#ifndef __BCBSPTREE_H__
#define __BCBSPTREE_H__

#include "BcPlane.h"

//////////////////////////////////////////////////////////////////////////
// Defines
#define MAX_BSP_VERTICES				32

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef std::vector< struct BcBSPNode* > BcBSPNodeList;

//////////////////////////////////////////////////////////////////////////
// BcBSPNode
struct BcBSPNode
{
	BcBSPNode*							pFront_;						///!< Front Node.
	BcBSPNode*							pBack_;							///!< Back node.

	BcPlane								Plane_;							///!< Plane dividing this node.
	std::vector< BcVec3d >				Vertices_;						///!< Vertices inside of this node.

	BcBSPNodeList						WorkingList_;					///!< List used during tree construction.
};

struct BcBSPInfo
{
	// Plane we intersect with.
	BcPlane								Plane_;

	//
	void								set( BcBSPNode* pNode );
};

struct BcBSPPointInfo
{
	BcPlane								Plane_;
	BcVec3d								Point_;
	BcReal								Distance_;
};

//-----------------------------------------------------------------------
///	@class BcBSPTree
/// @brief BSP Tree class.
/// @author Neil Richardson
///	@date [26/8/2008]
/// @version 1.0
///
/// BSP Tree container/builder class designed for collision detection
/// purposes. Could very easily be extended by virtualising BcBSPNode,
/// that way it could have its function expanded and become useful for
/// more things.
///
/// If the planes given describes a non-manifold object, then you may get
/// unexpected results in certain situations!
//-----------------------------------------------------------------------
class BcBSPTree
{
public:
	BcBSPTree();
	virtual ~BcBSPTree();

	/**
	*	Set a nodes plane and vertices.
	*	@param Plane Dividing plane of this node.
	*	@param pVertices Pointer to vertices for this node.
	*	@param nVertices Number of vertices.
	*/
	void								addNode( const BcPlane& Plane, const BcVec3d* pVertices, BcU32 nVertices );

	/**
	*	Build the tree.
	*/
	void								buildTree();

	/**
	*	Check if a point is infront of all required planes.
	*	@param Point Point to test.
	*	@param Radius Radius to test.
	*	@param Data to set for collisions.
	*	@param pNode Starting node.
	*/
	BcBool checkPointFront( const BcVec3d& Point, BcReal Radius, BcBSPInfo* pData = NULL, BcBSPNode* pNode = NULL );

	/**
	*	Check if a point is behind of all required planes.
	*	@param Point Point to test.
	*	@param Radius Radius to test.
	*	@param Data to set for collisions.
	*	@param pNode Starting node.
	*/
	BcBool checkPointBack( const BcVec3d& Point, BcReal Radius, BcBSPInfo* pData = NULL, BcBSPNode* pNode = NULL );

	/**
	*	Line intersection test.
	*/
	BcBool lineIntersection( const BcVec3d& A, const BcVec3d& B, BcBSPPointInfo* pPointInfo, BcBSPNode* pNode = NULL );

private:
	/**
	*	Classify a node against a plane.
	*	@param pNode Node to classify.
	*	@param Plane Plane to classify node against.
	*/
	BcPlane::eClassify					classifyNode( BcBSPNode* pNode, const BcPlane& Plane );

	/**
	*	Split node.
	*	@param pNode Node to split.
	*/
	void								splitNode( BcBSPNode* pNode );

	/**
	*	Clip a node and create a new node for it.	
	*	Clips off everything behind the specified plane.
	*	@param pNode Node to clip.
	*	@param Clip Clip plane.
	*	@return New clipped node.
	*/
	BcBSPNode*							clipNode( BcBSPNode* pNode, const BcPlane& Clip );

	/**
	*	Check if a point is in a node.
	*	@param Point Point to test.
	*	@param pNode Node to test again.
	*/
	BcBool								pointOnNode( const BcVec3d& Point, BcBSPNode* pNode );
	

	/**
	 *	Put a non-coinciding node to the front of the list.
	 */
	void								putNonCoincidingNodeAtTheFront( BcBSPNodeList& List );


private:
	BcBSPNodeList						NodeList_;
	BcBSPNode*							pRootNode_;
	BcU32								nInitialNodes_;
	BcU32								nTotalNodes_;

};

////////////////////////////////////////////////////////////////////////////////
// BcBSPData::set
inline void BcBSPInfo::set( BcBSPNode* pNode )
{
	BcAssert( pNode != NULL );
	Plane_ = pNode->Plane_;
}

#endif

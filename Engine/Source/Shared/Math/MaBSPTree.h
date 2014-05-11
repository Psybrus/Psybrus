/**************************************************************************
*
* File:		MaBSPTree.h
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

#include "MaPlane.h"

#include <vector>

//////////////////////////////////////////////////////////////////////////
// Defines
#define MAX_BSP_VERTICES				32

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef std::vector< struct MaBSPNode* > BcBSPNodeList;

//////////////////////////////////////////////////////////////////////////
// MaBSPNode
struct MaBSPNode
{
	MaBSPNode*							pFront_;						///!< Front Node.
	MaBSPNode*							pBack_;							///!< Back node.

	MaPlane								Plane_;							///!< Plane dividing this node.
	std::vector< MaVec3d >				Vertices_;						///!< Vertices inside of this node.

	BcBSPNodeList						WorkingList_;					///!< List used during tree construction.
};

struct MaBSPInfo
{
	// Plane we intersect with.
	MaPlane								Plane_;

	//
	void								set( MaBSPNode* pNode );
};

struct BcBSPPointInfo
{
	MaPlane								Plane_;
	MaVec3d								Point_;
	BcF32								Distance_;
};

//-----------------------------------------------------------------------
///	@class MaBSPTree
/// @brief BSP Tree class.
/// @author Neil Richardson
///	@date [26/8/2008]
/// @version 1.0
///
/// BSP Tree container/builder class designed for collision detection
/// purposes. Could very easily be extended by virtualising MaBSPNode,
/// that way it could have its function expanded and become useful for
/// more things.
///
/// If the planes given describes a non-manifold object, then you may get
/// unexpected results in certain situations!
//-----------------------------------------------------------------------
class MaBSPTree
{
public:
	MaBSPTree();
	virtual ~MaBSPTree();

	/**
	*	Set a nodes plane and vertices.
	*	@param Plane Dividing plane of this node.
	*	@param pVertices Pointer to vertices for this node.
	*	@param nVertices Number of vertices.
	*/
	void								addNode( const MaPlane& Plane, const MaVec3d* pVertices, BcU32 nVertices );

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
	BcBool checkPointFront( const MaVec3d& Point, BcF32 Radius, MaBSPInfo* pData = NULL, MaBSPNode* pNode = NULL );

	/**
	*	Check if a point is behind of all required planes.
	*	@param Point Point to test.
	*	@param Radius Radius to test.
	*	@param Data to set for collisions.
	*	@param pNode Starting node.
	*/
	BcBool checkPointBack( const MaVec3d& Point, BcF32 Radius, MaBSPInfo* pData = NULL, MaBSPNode* pNode = NULL );

	/**
	*	Line intersection test.
	*/
	BcBool lineIntersection( const MaVec3d& A, const MaVec3d& B, BcBSPPointInfo* pPointInfo, MaBSPNode* pNode = NULL );

private:
	/**
	*	Classify a node against a plane.
	*	@param pNode Node to classify.
	*	@param Plane Plane to classify node against.
	*/
	MaPlane::eClassify					classifyNode( MaBSPNode* pNode, const MaPlane& Plane );

	/**
	*	Split node.
	*	@param pNode Node to split.
	*/
	void								splitNode( MaBSPNode* pNode );

	/**
	*	Clip a node and create a new node for it.	
	*	Clips off everything behind the specified plane.
	*	@param pNode Node to clip.
	*	@param Clip Clip plane.
	*	@return New clipped node.
	*/
	MaBSPNode*							clipNode( MaBSPNode* pNode, const MaPlane& Clip );

	/**
	*	Check if a point is in a node.
	*	@param Point Point to test.
	*	@param pNode Node to test again.
	*/
	BcBool								pointOnNode( const MaVec3d& Point, MaBSPNode* pNode );
	

	/**
	 *	Put a non-coinciding node to the front of the list.
	 */
	void								putNonCoincidingNodeAtTheFront( BcBSPNodeList& List );


private:
	BcBSPNodeList						NodeList_;
	MaBSPNode*							pRootNode_;
	BcU32								nInitialNodes_;
	BcU32								nTotalNodes_;

};

#endif

/**************************************************************************
*
* File:		MdlBsp.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		BSP tree container
*		
*
*
* 
**************************************************************************/

#ifndef __MDLBSP_H__
#define __MDLBSP_H__

#include "MdlTypes.h"

#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// MdlBsp
class MdlBsp
{
public:
	MdlBsp();
	~MdlBsp();

	void allocatePool( BcU32 nNodes );

	//
	void addNode( const MdlBspNode& BspNode );

	//
	void build();

	//
	MdlBspNode* pRootNode() const;

	//
	BcU32 nNodes() const;

private:
	// Grab a new node from the pool.
	MdlBspNode* newNode();

	// Select a node to use as a dividing plane.
	MdlBspNode* bestFirstNode( MdlBspNodeList& NodeList );

	// Perform a split
	void split( MdlBspNode* pNode, MdlBspNodeList& NodeList );

	// Build indices for the nodes.
	void buildIndices( MdlBspNode* pNode );

	//
	enum eNodeClassify
	{
		bspNC_FRONT = 0,
		bspNC_BACK,
		bspNC_SPANNING
	};

	eNodeClassify classifyNode( MdlBspNode* pToClassify, MdlBspNode* pAgainst );

	//
	MdlBspNode* clipNode( MdlBspNode* pSourceNode, eNodeClassify ClassifyAs, const BcPlane& ClippingPlane );


private:
	MdlBspNode* pRoot_;
	BcU32 iIndex_;

	MdlBspNode* pNodePool_;
	BcU32 nNodes_;
	BcU32 iCurrentNode_;

	// We use this to work from.
	MdlBspNodeList NodeList_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline MdlBspNode* MdlBsp::newNode()
{
	BcAssert( iCurrentNode_ < nNodes_ );
	MdlBspNode* pNode = &pNodePool_[ iCurrentNode_++ ];
	return pNode;
}

inline MdlBspNode* MdlBsp::pRootNode() const
{
	return pRoot_;
}

inline BcU32 MdlBsp::nNodes() const
{
	return iIndex_;
}

#endif

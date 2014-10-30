/**************************************************************************
*
* File:		MdlBsp.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		BSP tree container
*		
*
*
* 
**************************************************************************/

#include "MdlBsp.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
MdlBsp::MdlBsp():
	pRoot_( NULL ),
	pNodePool_( NULL ),
	nNodes_( 0 ),
	iCurrentNode_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
MdlBsp::~MdlBsp()
{
	delete pNodePool_;
	pNodePool_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// allocatePool
void MdlBsp::allocatePool( BcU32 nNodes )
{
	delete pNodePool_;
	pNodePool_ = NULL;

	pNodePool_ = new MdlBspNode[ nNodes ];
	nNodes_ = nNodes;
}

//////////////////////////////////////////////////////////////////////////
// addNode
void MdlBsp::addNode( const MdlBspNode& BspNode )
{
	MdlBspNode* pNode = newNode();

	(*pNode) = BspNode;

	// Add to the list.
	NodeList_.push_back( pNode );
}

//////////////////////////////////////////////////////////////////////////
// build
void MdlBsp::build()
{
	// Select a node to do the first divide with
	pRoot_ = bestFirstNode( NodeList_ );

    // Now split the node recursively.
	split( pRoot_, NodeList_ );

	// Build indices
	if( pRoot_ != NULL )
	{
		iIndex_ = 0;
		buildIndices( pRoot_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// bestFirstNode
MdlBspNode* MdlBsp::bestFirstNode( MdlBspNodeList& NodeList )
{
	// TODO: Make it select a node which divides the tree as near in two as possible.
	MdlBspNodeList::iterator First = NodeList.begin();

	return (*First);
}

//////////////////////////////////////////////////////////////////////////
// split
void MdlBsp::split( MdlBspNode* pNode, MdlBspNodeList& NodeList )
{
	// Go through all faces in the list, classify them, and add them to the correct list for us.
	MdlBspNodeList::iterator Iter = NodeList.begin();

	while( Iter != NodeList.end() )
	{
		MdlBspNode* pCurrNode = (*Iter);

        if( pCurrNode != pNode )
		{
			eNodeClassify NodeClassification = classifyNode( pCurrNode, pNode );

			switch( NodeClassification )
			{
			case bspNC_FRONT:
				pNode->FrontList_.push_back( pCurrNode );
				break;

			case bspNC_BACK:
				pNode->BackList_.push_back( pCurrNode );
				break;

			case bspNC_SPANNING:
				{
					MdlBspNode* pFrontNode = clipNode( pCurrNode, bspNC_FRONT, pNode->Plane_ );
					MdlBspNode* pBackNode = clipNode( pCurrNode, bspNC_FRONT, pNode->Plane_ );

					pNode->FrontList_.push_back( pFrontNode );
					pNode->BackList_.push_back( pBackNode );
				}
				break;

			default:
				break;
			};
		}
		++Iter;
	}

	if( pNode->FrontList_.size() > 0 )
	{
		pNode->pFront_ = bestFirstNode( pNode->FrontList_ );

		if( pNode->pFront_ != NULL )
		{
			split( pNode->pFront_, pNode->FrontList_ );
		}
	}

	if( pNode->BackList_.size() > 0 )
	{
		pNode->pBack_ = bestFirstNode( pNode->BackList_ );

		if( pNode->pBack_ != NULL )
		{
			split( pNode->pBack_, pNode->BackList_ );
		}
	}

	NodeList.clear();
}

//////////////////////////////////////////////////////////////////////////
// buildIndices
void MdlBsp::buildIndices( MdlBspNode* pNode )
{
	pNode->Index_ = iIndex_;
	++iIndex_;

	if( pNode->pFront_ != NULL )
	{
		buildIndices( pNode->pFront_ );
	}

	if( pNode->pBack_ != NULL )
	{
		buildIndices( pNode->pBack_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// classifyNode
MdlBsp::eNodeClassify MdlBsp::classifyNode( MdlBspNode* pToClassify, MdlBspNode* pAgainst )
{
	// Count verts front and back.
	BcU32 nFront = 0;
	BcU32 nBack = 0;

	for( BcU32 i = 0; i < pToClassify->nVertices_; ++i )
	{
		if( pAgainst->Plane_.distance( pToClassify->Vertices_[ i ] ) < -1e3f )
		{
			++nBack;
		}
		else if( pAgainst->Plane_.distance( pToClassify->Vertices_[ i ] ) > 1e3f )
		{
			++nFront;
		}
	}

	// Front is favoured for nodes that sit on a plane.
	if( ( nFront > 0 && nBack == 0 ) || ( nFront == 0 && nBack == 0 ) )
	{
		return bspNC_FRONT;
	}
	else if( nFront == 0 && nBack > 0 )
	{
		return bspNC_BACK;
	}

	return bspNC_SPANNING;
}

//////////////////////////////////////////////////////////////////////////
// clipNode
MdlBspNode* MdlBsp::clipNode( MdlBspNode* pSourceNode, MdlBsp::eNodeClassify ClassifyAs, const MaPlane& ClippingPlane )
{
	// Add new vertices to list where required.
	std::vector< MaVec3d > VertexList;

	VertexList.reserve( 32 );

	for( BcU32 i = 0; i < pSourceNode->nVertices_; ++i )
	{
		const MaVec3d Start = pSourceNode->Vertices_[ i ];
		const MaVec3d End = pSourceNode->Vertices_[ ( i + 1 ) % pSourceNode->nVertices_ ];
		const MaVec3d Edge = ( End - Start );

		VertexList.push_back( Start );

		BcF32 Distance;

		if( ClippingPlane.lineIntersection( Start, Edge, Distance ) )
		{
			MaVec3d Intersection = Start - ( Edge * Distance );
			VertexList.push_back( Intersection );
		}
	}
	
	// Cull vertices that dont classify correctly
	MdlBspNode* pClippedNode = newNode();
	(*pClippedNode) = (*pSourceNode);

	BcU32 iDst = 0;
	const BcU32 nVertices = (BcU32)VertexList.size();

	for( BcU32 iSrc = 0; iSrc < nVertices; ++iSrc )
	{
		const MaVec3d& TestVert = VertexList[ iSrc ];
		const BcF32 Dist = ClippingPlane.distance( TestVert );

		switch( ClassifyAs )
		{
		case bspNC_FRONT:
			if( Dist > -0.0001f )
			{
				pClippedNode->Vertices_[ iDst ] = TestVert;
				++iDst;
			}
			break;

		case bspNC_BACK:
			if( Dist < 0.0001f )
			{
				pClippedNode->Vertices_[ iDst ] = TestVert;
				++iDst;
			}
			break;

		default:
			BcBreakpoint;
			break;
		}
	}

	pClippedNode->nVertices_ = iDst;

	BcAssert( pClippedNode->nVertices_ >= 3 );

	return pClippedNode;
}

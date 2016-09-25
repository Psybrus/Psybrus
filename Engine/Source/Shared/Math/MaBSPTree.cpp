/**************************************************************************
*
* File:		MaBSPTree.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		BSP Tree implementation.
*		
*
*
* 
**************************************************************************/

#include "Math/MaBSPTree.h"

#include "Base/BcDebug.h"
#include "Base/BcMath.h"

////////////////////////////////////////////////////////////////////////////////
// BcBSPData::set
void MaBSPInfo::set( MaBSPNode* pNode )
{
	BcAssert( pNode != NULL );
	Plane_ = pNode->Plane_;
}

////////////////////////////////////////////////////////////////////////////////
// Constructor
MaBSPTree::MaBSPTree():
	pRootNode_( NULL )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
//virtual
MaBSPTree::~MaBSPTree()
{
	for( BcU32 Idx = 0; Idx < NodeList_.size(); ++Idx )
	{
		delete NodeList_[ Idx ];
	}
	NodeList_.clear();
	pRootNode_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// addNode
void MaBSPTree::addNode( const MaPlane& Plane, const MaVec3d* pVertices, BcU32 nVertices )
{
	MaBSPNode* pNewNode = new MaBSPNode();

	pNewNode->pFront_ = NULL;
	pNewNode->pBack_ = NULL;
	pNewNode->Plane_ = Plane;
	
	for( BcU32 i = 0; i < nVertices; ++i )
	{
		// Debug check, check point is coplanar.
		BcAssertMsg( Plane.classify( pVertices[ i ] ) == MaPlane::bcPC_COINCIDING, "Point is non-coplanar." );

		// Set vertex.
		pNewNode->Vertices_.push_back( pVertices[ i ] );
	}
	
	NodeList_.push_back( pNewNode );
}

////////////////////////////////////////////////////////////////////////////////
// buildTree
void MaBSPTree::buildTree()
{
	// Set root node.
	if( NodeList_.size() > 0 )
	{
		// Put non coinciding node at the front.
		putNonCoincidingNodeAtTheFront( NodeList_ );

		// Grab first for split.
		pRootNode_ = NodeList_[ 0 ];

		// Put all items into root node for splitting.
		for( BcU32 i = 1; i < NodeList_.size(); ++i )
		{
			pRootNode_->WorkingList_.push_back( NodeList_[ i ] );
		}

		// Split.
		splitNode( pRootNode_ );
	}
}

////////////////////////////////////////////////////////////////////////////////
// checkPointFront
BcBool MaBSPTree::checkPointFront( const MaVec3d& Point, BcF32 Radius, MaBSPInfo* pData /*= NULL*/, MaBSPNode* pNode /*= NULL */ )
{
	// Check if we want the root node.
	if( pNode == NULL )
	{
		pNode = pRootNode_;
	}

	// Classify against plane
	if( pNode->Plane_.classify( Point, Radius ) != MaPlane::bcPC_FRONT )
	{
		// Behind plane.
		if( pNode->pBack_ == NULL )
		{
			if( pData )
			{
				pData->set( pNode );
			}

			return BcFalse;
		}
		else
		{
			return checkPointFront( Point, Radius, pData, pNode->pBack_ );
		}
	}
	else
	{
		// Infront of plane.
		if( pNode->pFront_ == NULL )
		{
			if( pData )
			{
				pData->set( pNode );
			}

			return BcTrue;
		}
		else
		{
			return checkPointFront( Point, Radius, pData, pNode->pFront_ );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// checkPointBack
BcBool MaBSPTree::checkPointBack( const MaVec3d& Point, BcF32 Radius, MaBSPInfo* pData /*= NULL*/, MaBSPNode* pNode /*= NULL*/ )
{
	return !checkPointFront( Point, Radius, pData, pNode );
}

////////////////////////////////////////////////////////////////////////////////
// findPointOnEdge
BcBool MaBSPTree::lineIntersection( const MaVec3d& A, const MaVec3d& B, BcBSPPointInfo* pPointInfo, MaBSPNode* pNode /*= NULL*/ )
{
	// Check if we want the root node.
	if( pNode == NULL )
	{
		pNode = pRootNode_;
		if( pPointInfo != NULL )
		{
			pPointInfo->Point_ = B;
			pPointInfo->Distance_ = 1e12f;
		}
	}

	// Really naive way...
	BcBool Intersected = BcFalse;

	// TODO: Handle coincident intersection!
	MaPlane::eClassify ClassifyA = pNode->Plane_.classify( A, 0.0f );
	MaPlane::eClassify ClassifyB = pNode->Plane_.classify( B, 0.0f );

	if( ClassifyA == MaPlane::bcPC_FRONT && ClassifyB == MaPlane::bcPC_BACK )
	{
		BcF32 T;
		MaVec3d Intersection;
		if( pNode->Plane_.lineIntersection( A, B, T, Intersection ) )
		{
			// Is point in vertices? If so go down front to find nearer intersection.
			if( pointOnNode( Intersection, pNode ) )
			{
				// If we want point info recurse deeper to get it.
				if( pPointInfo != NULL )
				{
					BcF32 DistanceSquared = ( A - Intersection ).magnitudeSquared();

					// Check the distance, if it's less then go deeper!
					if( DistanceSquared < ( pPointInfo->Distance_ * pPointInfo->Distance_ ) )
					{
						pPointInfo->Plane_ = pNode->Plane_;
						pPointInfo->Distance_ = BcSqrt( DistanceSquared );
						pPointInfo->Point_ = Intersection;
					}
				}

				Intersected |= BcTrue;
			}
		}
	}
	
	// NOTE: ClassifyB shouldn't be required here ...right?
	if( pNode->pFront_ != NULL )
	{
		if( ClassifyA == MaPlane::bcPC_FRONT || ClassifyB == MaPlane::bcPC_FRONT )
		{
			Intersected |= lineIntersection( A, B, pPointInfo, pNode->pFront_ );
		}
	}

	if( pNode->pBack_ != NULL )
	{
		if( ClassifyA == MaPlane::bcPC_BACK || ClassifyB == MaPlane::bcPC_BACK )
		{
			Intersected |= lineIntersection( A, B, pPointInfo, pNode->pBack_ );
		}
	}

	return Intersected;
}

////////////////////////////////////////////////////////////////////////////////
// classifyNode
MaPlane::eClassify MaBSPTree::classifyNode( MaBSPNode* pNode, const MaPlane& Plane )
{
	//Grab result of first test
	MaPlane::eClassify FirstResult = Plane.classify( pNode->Vertices_[0] );

	//Make sure remaining tests produce the same result
	for( BcU32 iNode = 0; iNode < pNode->Vertices_.size(); ++iNode)
	{
		MaPlane::eClassify Result = Plane.classify( pNode->Vertices_[ iNode ], 1e-3f );

		// On as a first result is not useful to us.
		if( FirstResult == MaPlane::bcPC_COINCIDING && Result != MaPlane::bcPC_COINCIDING )
		{
			FirstResult = Result;
		}

		if( Result != FirstResult && Result != MaPlane::bcPC_COINCIDING )
		{
			//Different result for test -> bail out
			return MaPlane::bcPC_SPANNING;
		}
	}

	// Success
	return FirstResult;
}

////////////////////////////////////////////////////////////////////////////////
// splitNode
void MaBSPTree::splitNode( MaBSPNode* pNode )
{
	MaBSPNode* pSwappedNode = NULL;

	BcBSPNodeList FList_;			// Front list.
	BcBSPNodeList BList_;			// Back list.

	// Move non coinciding node to the front.
	putNonCoincidingNodeAtTheFront( pNode->WorkingList_ );

	// Split up this nodes list into a front and back list.
	for( BcBSPNodeList::iterator It( pNode->WorkingList_.begin() ); It != pNode->WorkingList_.end(); )
	{
		MaPlane::eClassify Classify = classifyNode( (*It), pNode->Plane_ );

		// If it's coinciding use normal to determine facing.
		if( Classify == MaPlane::bcPC_COINCIDING )
		{
			/*
			if( pNode->Plane_.normal().dot( (*It)->Plane_.normal() ) < 0.0f )
			{
				Classify = MaPlane::bcPC_FRONT;
			}
			else
			{
				Classify = MaPlane::bcPC_FRONT;
			}
			*/
			// HACK, seems to work...just:
			Classify = MaPlane::bcPC_FRONT;
		}

		// Classify the node.
		switch( Classify )
		{
		case MaPlane::bcPC_FRONT:
			{
				// Node is entirely infront.
				pSwappedNode = (*It);				
				It = pNode->WorkingList_.erase( It );
				FList_.push_back( pSwappedNode );
				BcAssert( pSwappedNode->Vertices_.size() >= 2 );
			}
			break;

		case MaPlane::bcPC_BACK:
			{
				// Node is entirely behind.
				pSwappedNode = (*It);
				It = pNode->WorkingList_.erase( It );
				BList_.push_back( pSwappedNode );
				BcAssert( pSwappedNode->Vertices_.size() >= 2 );
			}
			break;

		case MaPlane::bcPC_SPANNING:
			{
				// Node spans the plane. Must be clipped.
				MaBSPNode* pFront = clipNode( (*It), MaPlane( pNode->Plane_.normal().x(), pNode->Plane_.normal().y(), pNode->Plane_.normal().z(), pNode->Plane_.d() ) );
				MaBSPNode* pBack = clipNode( (*It), MaPlane( -pNode->Plane_.normal().x(), -pNode->Plane_.normal().y(), -pNode->Plane_.normal().z(), -pNode->Plane_.d() ) );

				// Original node can be dropped.
				pSwappedNode = (*It);
				It = pNode->WorkingList_.erase( It );			
				
				// Add new nodes to lists.
				FList_.push_back( pFront );
				BcAssert( pFront->Vertices_.size() >= 2 );

				BList_.push_back( pBack );
				BcAssert( pBack->Vertices_.size() >= 2 );
			}
			break;

		default:
			//Bc_AssertFailMsg( "Duplicate plane in tree." );
			BcBreakpoint;
			break;
		}
	}
	
	// Select a front and back node for each list.
	if( FList_.size() > 0 )
	{
		pNode->pFront_ = (*FList_.begin());
		FList_.erase( FList_.begin() );

		pSwappedNode = nullptr;
		for( BcBSPNodeList::iterator It( FList_.begin() ); It != FList_.end(); )
		{
			pSwappedNode = (*It);
			It = FList_.erase( It );
			pNode->pFront_->WorkingList_.push_back( pSwappedNode );
			BcAssert( pSwappedNode->Vertices_.size() >= 2 );
		}

		splitNode( pNode->pFront_ );
	}

	if( BList_.size() > 0 )
	{
		pNode->pBack_ = (*BList_.begin());
		BList_.erase( BList_.begin() );

		pSwappedNode = nullptr;
		for( BcBSPNodeList::iterator It( BList_.begin() ); It != BList_.end(); )
		{
			pSwappedNode = (*It);
			It = BList_.erase( It );
			pNode->pBack_->WorkingList_.push_back( pSwappedNode );
			BcAssert( pSwappedNode->Vertices_.size() >= 2 );
		}

		splitNode( pNode->pBack_ );
	}


}

////////////////////////////////////////////////////////////////////////////////
// clipNode
MaBSPNode* MaBSPTree::clipNode( MaBSPNode* pNode, const MaPlane& Clip )
{
	MaBSPNode* pNewNode = new MaBSPNode();

	pNewNode->pFront_ = NULL;
	pNewNode->pBack_ = NULL;
	pNewNode->Plane_ = pNode->Plane_;

	std::vector< MaVec3d > Vertices;

	for( BcU32 iVert = 0; iVert < pNode->Vertices_.size(); ++iVert )
	{
		MaVec3d PointA = pNode->Vertices_[ ( iVert ) ];
		MaVec3d PointB = pNode->Vertices_[ ( iVert + 1 ) % pNode->Vertices_.size() ];
		MaVec3d Intersection;
		BcF32 Distance;

		//
		Vertices.push_back( PointA );

		// Check for intersection.
		if( Clip.lineIntersection( PointA, PointB, Distance, Intersection ) )
		{
			Vertices.push_back( Intersection );
		}
	}

	//
	BcAssertMsg( Vertices.size() >= 2, "Not enough vertices emitted." );

	// Classify all these points and add correct ones to new node.
	for( BcU32 iVert = 0; iVert < Vertices.size(); ++iVert )
	{
		if( Clip.classify( Vertices[ iVert ] ) != MaPlane::bcPC_BACK )
		{
			pNewNode->Vertices_.push_back( Vertices[ iVert ] );
		}
	}

	BcAssertMsg( pNewNode->Vertices_.size() >= 2, "Not enough vertices emitted." );

	return pNewNode;
}

////////////////////////////////////////////////////////////////////////////////
// pointOnNode
BcBool MaBSPTree::pointOnNode( const MaVec3d& Point, MaBSPNode* pNode )
{
	// Check if point is coincident.
	if( pNode->Plane_.classify( Point, 1e-3f ) != MaPlane::bcPC_COINCIDING )
	{
		return BcFalse;
	}

	//
	if( pNode->Vertices_.size() == 2 )
	{
		// If we only have 2 verts, test we are on the defining line.
		MaVec3d Origin = pNode->Vertices_[ 0 ];
		MaVec3d Direction = pNode->Vertices_[ 1 ] - pNode->Vertices_[ 0 ];
		MaVec3d Vector = Point - Origin;
		BcF32 T = Direction.dot( Vector );

		// Check its on the line.
		return ( T >= 0.0f ) && ( T <= 1.0f );
	}
	else
	{
		BcS32 Sign = 0;
		for( BcU32 Idx = 0; Idx < pNode->Vertices_.size(); ++Idx )
		{
			const MaVec3d& PointA = pNode->Vertices_[ Idx ];
			const MaVec3d& PointB = pNode->Vertices_[ ( Idx + 1 ) % pNode->Vertices_.size() ];
			const MaVec3d AffineSegment = PointB - PointA;
			const MaVec3d AffinePoint = Point - PointA;
			BcF32 K = AffineSegment.dot( AffinePoint );
			BcS32 KSign = BcAbs( K ) > 1e-3f ? static_cast< BcS32 >( K / BcAbs( K ) ) : 0;
			if( Sign == 0 )
			{
				Sign = KSign;
			}
			else if( KSign != Sign )
			{
				return BcFalse;
			}
		}

		return BcTrue;
	}
}

////////////////////////////////////////////////////////////////////////////////
// putNonCoincidingNodeAtTheFront
void MaBSPTree::putNonCoincidingNodeAtTheFront( BcBSPNodeList& List )
{
	for( BcU32 TotalIdx = 0; TotalIdx < List.size(); ++TotalIdx )
	{
		// Get first node.
		MaBSPNode* pNode = List[ 0 ];

		// Check if it's coinciding with any other in the list (except itself).
		BcBool IsCoinciding = BcFalse;
		for( BcU32 Idx = 1; Idx < List.size(); ++Idx )
		{
			// Classify the node.
			if( classifyNode( List[ Idx ], pNode->Plane_ ) == MaPlane::bcPC_COINCIDING )
			{
				IsCoinciding = BcTrue;
			}
		}
		
		// If coinciding, put at the back and repeat, otherwise we can bail.
		if( IsCoinciding )
		{
			// Put to the end of the list.
			List.erase( List.begin() );
			List.push_back( pNode );
		}
		else
		{
			break;
		}
	}
}

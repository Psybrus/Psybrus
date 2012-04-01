/**************************************************************************
*
* File:		MdlNode.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Model node.
*		
*
*
* 
**************************************************************************/

#include "MdlNode.h"

#include "MdlMesh.h"
#include "MdlMorph.h"
#include "MdlEntity.h"

#include "Base/BcDebug.h"
#include "Base/BcString.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
MdlNode::MdlNode():
	NodeType_( eNT_EMPTY ),
	pParent_( NULL ),
	pChild_( NULL ),
	pNext_( NULL )
{
	RelativeTransform_.identity();
	AbsoluteTransform_.identity();

	pNodeMeshObject_ = NULL;
	pNodeSkinObject_ = NULL;
	pNodeColMeshObject_ = NULL;
	pNodeMorphObject_ = NULL;
	pNodeEntityObject_ = NULL;
	pNodeLightObject_ = NULL;
	pNodeProjectorObject_ = NULL;
}


//////////////////////////////////////////////////////////////////////////
// Dtor
MdlNode::~MdlNode()
{
	delete pNodeMeshObject_;
	delete pNodeSkinObject_;
	delete pNodeColMeshObject_;
	delete pNodeMorphObject_;
	delete pNodeEntityObject_;
	delete pNodeLightObject_;
	delete pNodeProjectorObject_;

	///* NOTE: Memory corruption somewhere.

	MdlNode* pNext = pChild_;
	while( pNext != NULL )
	{
		MdlNode* pTheNext = pNext->pNext_;
		delete pNext;
		pNext = pTheNext;
	}
}

//////////////////////////////////////////////////////////////////////////
// name
void MdlNode::name( const BcChar* Name )
{
	BcStrCopy( Name_, Name );
}

const BcChar* MdlNode::name()
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// parentNode
BcBool MdlNode::parentNode( MdlNode* pNode, const BcChar* ParentName )
{
	BcBool bParented = BcFalse;

	// Parent another node to this tree.
	if( ParentName == NULL || BcStrCompare( ParentName, Name_ ) )
	{
		// Tell the node who its parent is.
		pNode->pParent_ = this;

		// If we have a child already we need our next to match.
		if( pChild_ != NULL )
		{
			MdlNode* pParentNode = pChild_;

			// Find the last node
			while( pParentNode->pNext_ != NULL )
			{
				BcAssert( pParentNode != pParentNode->pNext_ );
				pParentNode = pParentNode->pNext_;
			}

			//
			pParentNode->pNext_ = pNode;

			bParented = BcTrue;
		}
		else
		{
			pChild_ = pNode;
			bParented = BcTrue;
		}
	}
	else
	{
		// If we are not the destined parent pass it to next and child.
		// Find the last node
		MdlNode* pNextNode = pChild_;

		while( pNextNode != NULL )
		{
			bParented |= pNextNode->parentNode( pNode, ParentName );
			pNextNode = pNextNode->pNext_;
		}
	}

	return bParented;
}

//////////////////////////////////////////////////////////////////////////
// makeRelativeTransform
void MdlNode::makeRelativeTransform( const BcMat4d& ParentAbsolute )
{
	// Parent another node to this tree.
	MdlNode* pNextNode = pChild_;

	//
	BcMat4d InverseParent = ParentAbsolute;
	InverseParent.inverse();

	RelativeTransform_ = AbsoluteTransform_ * InverseParent;

	// For all the nodes parented to the same as us.
	while( pNextNode != NULL )
	{
		pNextNode->makeRelativeTransform( AbsoluteTransform_ );
		pNextNode = pNextNode->pNext_;
	}
}

//////////////////////////////////////////////////////////////////////////
// makeAbsoluteTransform
void MdlNode::makeAbsoluteTransform( const BcMat4d& ParentAbsolute )
{
	// Parent another node to this tree.
	MdlNode* pNextNode = pChild_;

	//
	BcMat4d InverseParent = ParentAbsolute;
	InverseParent.inverse();

	AbsoluteTransform_ = ParentAbsolute * RelativeTransform_;

	// For all the nodes parented to the same as us.
	while( pNextNode != NULL )
	{
		pNextNode->makeAbsoluteTransform( AbsoluteTransform_ );
		pNextNode = pNextNode->pNext_;
	}
}

//////////////////////////////////////////////////////////////////////////
// flipTransform
void MdlNode::flipTransform( BcMat4d& Transform )
{
	Transform.transpose();
	Transform[0][2] = -Transform[0][2];
	Transform[1][2] = -Transform[1][2];
	Transform[2][0] = -Transform[2][0];
	Transform[2][1] = -Transform[2][1];
	Transform[2][3] = -Transform[2][3];
	Transform.transpose();
}

//////////////////////////////////////////////////////////////////////////
// flipTransforms
void MdlNode::flipCoordinateSpace()
{
	// Parent another node to this tree.
	MdlNode* pNextNode = pChild_;

	// Flip absolute and inverse bind pose transforms.
	flipTransform( AbsoluteTransform_ );
	flipTransform( InverseBindpose_ );

	// If we've got a mesh, flip it's coordinate space.
	if( type() & eNT_MESH )
	{
		pNodeMeshObject_->flipCoordinateSpace();
	}
	
	if( type() & eNT_SKIN )
	{
		pNodeSkinObject_->flipCoordinateSpace();
	}

	if( type() & eNT_MORPH )
	{
		pNodeMorphObject_->flipCoordinateSpace();
	}

	// For all the nodes parented to the same as us.
	while( pNextNode != NULL )
	{
		pNextNode->flipCoordinateSpace();
		pNextNode = pNextNode->pNext_;
	}
}

//////////////////////////////////////////////////////////////////////////
// type
void MdlNode::type( BcU32 NodeType )
{
	if( NodeType != eNT_COLMESH && NodeType != eNT_ENTITY )
	{
		BcAssert( ( NodeType_ & ~NodeType ) == 0 );
	}

	if( ( NodeType_ & NodeType ) == 0 )
	{
		NodeType_ |= NodeType;

		switch( NodeType )
		{
		case eNT_EMPTY:
			break;

		case eNT_MESH:
			pNodeMeshObject_ = new MdlMesh();
			break;

		case eNT_SKIN:
			pNodeSkinObject_ = new MdlMesh();
			break;

		case eNT_COLMESH:
			// Naughty.
			//pNodeColMeshObject_ = new MdlMesh();
			BcAssert( pNodeMeshObject_ != NULL );
			break;

		case eNT_MORPH:
			pNodeMorphObject_ = new MdlMesh();
			break;

		case eNT_ENTITY:
			pNodeEntityObject_ = new MdlEntity();
			break;

		case eNT_LIGHT:
			pNodeLightObject_ = new MdlLight();
			break;

		case eNT_PROJECTOR:
			pNodeProjectorObject_ = new MdlProjector();
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// countJoints
void MdlNode::countJoints( BcU32& iCount )
{
	if( type() & eNT_JOINT )
	{
		iCount++;
	}

	MdlNode* pNextNode = pChild_;
	while( pNextNode != NULL )
	{
		if( pNextNode->type() & eNT_JOINT )
		{
			pNextNode->countJoints( iCount );
		}

		pNextNode = pNextNode->pNext_;
	}
}

//////////////////////////////////////////////////////////////////////////
//
void MdlNode::findAllAABBs() 
{
	MdlNode* pNextNode = pChild_;

	// If our AABB is empty, calculate our bounds based on child nodes.
	if( AABB_.isEmpty() )
	{
		while( pNextNode != NULL )
		{
			// Find our childs..
			pNextNode->findAllAABBs();

			// Add its AABB to ours.
			if( pNextNode->AABB_.isEmpty() == BcFalse )
			{
				AABB_.expandBy( pNextNode->AABB_ );
			}

			//
			pNextNode = pNextNode->pNext_;
		}

		// Transform our AABB
		if( AABB_.isEmpty() == BcFalse )
		{
			AABB_ = AABB_.transform( AbsoluteTransform_ );
		}
	}
	else
	{
		// Transform ours.
		AABB_ = AABB_.transform( AbsoluteTransform_ );
	}
}

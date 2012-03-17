/**************************************************************************
*
* File:		MdlNode.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Model node.
*		
*
*
* 
**************************************************************************/

#ifndef __MDLNODE_H__
#define __MDLNODE_H__

#include "MdlTypes.h"

#include "BcMat4d.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class MdlMesh;
class MdlMorph;
class MdlEntity;

//////////////////////////////////////////////////////////////////////////
// MdlNode
class MdlNode
{
public:
	MdlNode();
	~MdlNode();

	void name( const BcChar* Name );
	const BcChar* name();

	BcBool parentNode( MdlNode* pNode, const BcChar* ParentName );

	void makeRelativeTransform( const BcMat4d& ParentAbsolute );
	void makeAbsoluteTransform( const BcMat4d& ParentAbsolute );

	void flipTransform( BcMat4d& Transform );
	void flipCoordinateSpace();

	void type( BcU32 NodeType );
	BcU32 type() const;

	void relativeTransform( const BcMat4d& Transform );
	const BcMat4d& relativeTransform();

	void absoluteTransform( const BcMat4d& Transform );
	const BcMat4d& absoluteTransform();

	void inverseBindpose( const BcMat4d& Transform );
	const BcMat4d& inverseBindpose();

	void aabb( const BcAABB& AABB );
	const BcAABB& aabb() const;

	void findAllAABBs();

	void countJoints( BcU32& iCount );

	MdlMesh* pMeshObject();
	MdlMesh* pSkinObject();
	MdlMesh* pColMeshObject();
	MdlMesh* pMorphObject();
	MdlEntity* pEntityObject();
	MdlLight* pLightObject();
	MdlProjector* pProjectorObject();

	MdlNode* pParent();
	MdlNode* pChild();
	MdlNode* pNext();

private:
	//
	BcChar				Name_[ MDL_NODENAME_LEN ];

	// Node object types
	BcU32				NodeType_;

	//
	MdlMesh*			pNodeMeshObject_;
	MdlMesh*			pNodeSkinObject_;
	MdlMesh*			pNodeColMeshObject_;
	MdlMesh*			pNodeMorphObject_;
	MdlEntity*			pNodeEntityObject_;
	MdlLight*			pNodeLightObject_;
	MdlProjector*		pNodeProjectorObject_;

	//
	MdlNode*			pParent_;
	MdlNode*			pChild_;
	MdlNode*			pNext_;

	BcMat4d				RelativeTransform_;
	BcMat4d				AbsoluteTransform_;
	BcMat4d				InverseBindpose_;

	BcAABB				AABB_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline BcU32 MdlNode::type() const
{
	return NodeType_;
}

inline MdlMesh* MdlNode::pMeshObject()
{
	return pNodeMeshObject_;
}

inline MdlMesh* MdlNode::pSkinObject()
{
	return pNodeSkinObject_;
}

inline MdlMesh* MdlNode::pColMeshObject()
{
	return pNodeMeshObject_;
	//return pNodeColMeshObject_;
}

inline MdlMesh* MdlNode::pMorphObject()
{
	return pNodeMorphObject_;
}

inline MdlEntity* MdlNode::pEntityObject()
{
	return pNodeEntityObject_;
}

inline MdlLight* MdlNode::pLightObject()
{
	return pNodeLightObject_;
}

inline MdlProjector* MdlNode::pProjectorObject()
{
	return pNodeProjectorObject_;
}

inline void MdlNode::relativeTransform( const BcMat4d& RelativeTransform )
{
	RelativeTransform_ = RelativeTransform;
}

inline const BcMat4d& MdlNode::relativeTransform()
{
	return RelativeTransform_;
}

inline void MdlNode::absoluteTransform( const BcMat4d& AbsoluteTransform )
{
	AbsoluteTransform_ = AbsoluteTransform;
}

inline const BcMat4d& MdlNode::absoluteTransform()
{
	return AbsoluteTransform_;
}

inline void MdlNode::inverseBindpose( const BcMat4d& InverseBindpose )
{
	InverseBindpose_ = InverseBindpose;
}

inline const BcMat4d& MdlNode::inverseBindpose()
{
	return InverseBindpose_;
}

inline void MdlNode::aabb( const BcAABB& AABB )
{
	AABB_ = AABB;
}

inline const BcAABB& MdlNode::aabb() const
{
	return AABB_;
}

inline	MdlNode* MdlNode::pParent()
{
	return pParent_;
}

inline	MdlNode* MdlNode::pChild()
{
	return pChild_;
}

inline	MdlNode* MdlNode::pNext()
{
	return pNext_;
}

#endif

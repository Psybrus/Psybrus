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

#include "Math/MaMat4d.h"

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

	void makeRelativeTransform( const MaMat4d& ParentAbsolute );
	void makeAbsoluteTransform( const MaMat4d& ParentAbsolute );

	void flipTransform( MaMat4d& Transform );
	void flipCoordinateSpace();

	void type( BcU32 NodeType );
	BcU32 type() const;

	void relativeTransform( const MaMat4d& Transform );
	const MaMat4d& relativeTransform();

	void absoluteTransform( const MaMat4d& Transform );
	const MaMat4d& absoluteTransform();

	void inverseBindpose( const MaMat4d& Transform );
	const MaMat4d& inverseBindpose();

	void aabb( const MaAABB& AABB );
	const MaAABB& aabb() const;

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

	MaMat4d				RelativeTransform_;
	MaMat4d				AbsoluteTransform_;
	MaMat4d				InverseBindpose_;

	MaAABB				AABB_;
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

inline void MdlNode::relativeTransform( const MaMat4d& RelativeTransform )
{
	RelativeTransform_ = RelativeTransform;
}

inline const MaMat4d& MdlNode::relativeTransform()
{
	return RelativeTransform_;
}

inline void MdlNode::absoluteTransform( const MaMat4d& AbsoluteTransform )
{
	AbsoluteTransform_ = AbsoluteTransform;
}

inline const MaMat4d& MdlNode::absoluteTransform()
{
	return AbsoluteTransform_;
}

inline void MdlNode::inverseBindpose( const MaMat4d& InverseBindpose )
{
	InverseBindpose_ = InverseBindpose;
}

inline const MaMat4d& MdlNode::inverseBindpose()
{
	return InverseBindpose_;
}

inline void MdlNode::aabb( const MaAABB& AABB )
{
	AABB_ = AABB;
}

inline const MaAABB& MdlNode::aabb() const
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

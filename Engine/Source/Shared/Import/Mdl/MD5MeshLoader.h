/**************************************************************************
*
* File:		MD5MeshLoader.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __MD5MESHLOADER_H__
#define __MD5MESHLOADER_H__

#include "BcVectors.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class MdlNode;
class MdlMesh;

//////////////////////////////////////////////////////////////////////////
// Structures

struct MD5_Joint
{
	BcU32 ParentID_;
	BcChar Name_[32];

	// Mesh specific.
	BcF32 TX_, TY_, TZ_;
	BcF32 QX_, QY_, QZ_;

	// Anim specific.
	BcU32 AnimMask_;
	BcU32 AnimOffset_;
};

struct MD5_Vert
{
	BcU32 Index_;
	BcF32 U_, V_;
	BcU32 WeightIndex_;
	BcU32 nWeights_;
};

struct MD5_Weight
{
	BcU32 Index_;
	BcU32 JointID_;
	BcF32 Weight_;
	BcF32 X_, Y_, Z_;
};

struct MD5_Mesh
{
	BcChar Shader_[256];

	BcU32 nIndices_;
	BcU32* pIndices_;

	BcU32 nVerts_;
	MD5_Vert* pVerts_;

	BcU32 nWeights_;
	MD5_Weight* pWeights_;
};

//////////////////////////////////////////////////////////////////////////
// MD5MeshLoader
class MD5MeshLoader
{
public:
	MD5MeshLoader();
	~MD5MeshLoader();

	MdlNode* load( const BcChar* FileName, const BcChar* NodeName );

	void buildBindPose( MdlNode* pNode, BcU32 iMesh );

private:
	BcU32 nJoints() const;
	MD5_Joint* pJoint( BcU32 Index );

	BcU32 nMeshes() const;
	MD5_Mesh* pMesh( BcU32 Index );

private:
	enum eParseMode
	{
		PM_MAIN = 0,
		PM_JOINTS,
		PM_MESH
	};

	eParseMode ParseMode_;

	BcU32 nJoints_;
	MD5_Joint* pJoints_;

	BcU32 nMeshes_;
	MD5_Mesh* pMeshes_;
};

//////////////////////////////////////////////////////////////////////////
// Inlines

inline BcU32 MD5MeshLoader::nJoints() const
{
	return nJoints_;
}

inline MD5_Joint* MD5MeshLoader::pJoint( BcU32 Index )
{
	return &pJoints_[ Index ];
}

inline BcU32 MD5MeshLoader::nMeshes() const
{
	return nMeshes_;
}

inline MD5_Mesh* MD5MeshLoader::pMesh( BcU32 Index )
{
	return &pMeshes_[ Index ];
}

#endif

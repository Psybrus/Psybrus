/**************************************************************************
*
* File:		Rendering/ScnModelFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMODELFILEDATA__
#define __SCNMODELFILEDATA__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsUniquePointers.h"
#include "System/Scene/Rendering/ScnMaterial.h" // TODO: Get rid of this reference.

//////////////////////////////////////////////////////////////////////////
// Defines
#define SCN_MODEL_BONE_PALETTE_SIZE		( ScnShaderBoneUniformBlockData::MAX_BONES )

//////////////////////////////////////////////////////////////////////////
// ScnModelHeader
struct ScnModelHeader
{
	BcU32 NoofNodes_;
	BcU32 NoofPrimitives_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnModelNodeTransformData
struct ScnModelNodeTransformData
{
	MaMat4d LocalTransform_;
	MaMat4d WorldTransform_;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelNodePropertyData
struct ScnModelNodePropertyData
{
	BcU32 ParentIndex_;
	BcName Name_;
	BcBool IsBone_;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelMeshData
struct ScnModelMeshData
{
	BcU32 NodeIndex_;
	BcBool IsSkinned_;
	RsTopologyType Type_;
	ScnShaderPermutationFlags ShaderPermutation_;
	BcU32 NoofIndices_;
	BcU32 MaterialRef_;
	BcU32 Padding0_;
	BcU32 Padding1_;
	MaAABB AABB_;
	BcU32 NoofVertices_;
	BcU32 NoofVertexElements_;
	BcU32 VertexStride_;
	union
	{
		BcU64 VertexElementsId_;
		RsVertexElement* VertexElements_;
	};
	BcU32 BonePalette_[ SCN_MODEL_BONE_PALETTE_SIZE ];
	MaMat4d BoneInverseBindpose_[ SCN_MODEL_BONE_PALETTE_SIZE ];
};

//////////////////////////////////////////////////////////////////////////
// ScnModelMeshRuntime
struct ScnModelMeshRuntime
{
	BcU32 MeshDataIndex_ = BcErrorCode;
	RsGeometryBinding* GeometryBinding_ = nullptr;
	size_t VertexBufferOffset_ = 0;
	size_t IndexBufferOffset_ = 0;
	ScnMaterialRef MaterialRef_ = nullptr; // TODO: Don't use ref, just use pointer.
};

//////////////////////////////////////////////////////////////////////////
// ScnModelMeshRuntimeList: TODO: Just change to a static array in code.
typedef std::vector< ScnModelMeshRuntime > ScnModelMeshRuntimeList;

#endif

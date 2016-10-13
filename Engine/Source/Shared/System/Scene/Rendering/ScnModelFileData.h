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

#include "System/Content/CsTypes.h"
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
	MaAABB AABB_;
	BcU32 NodeIdx_;
	BcBool IsSkinned_;
	ScnShaderPermutationFlags ShaderPermutation_;
	BcU32 DrawIdx_;
	CsCrossRefId RenderMeshRef_;
	CsCrossRefId MaterialRef_;
	BcU32 Padding0_;
	BcU32 Padding1_;
	BcU32 BonePalette_[ SCN_MODEL_BONE_PALETTE_SIZE ];
	MaMat4d BoneInverseBindpose_[ SCN_MODEL_BONE_PALETTE_SIZE ];
};

//////////////////////////////////////////////////////////////////////////
// ScnModelMeshRuntime
struct ScnModelMeshRuntime
{
	BcU32 MeshDataIdx_ = 0;
	class ScnRenderMesh* RenderMeshRef_ = nullptr;
	class ScnMaterial* MaterialRef_ = nullptr;
};

//////////////////////////////////////////////////////////////////////////
// ScnModelMeshRuntimeList: TODO: Just change to a static array in code.
typedef std::vector< ScnModelMeshRuntime > ScnModelMeshRuntimeList;

#endif

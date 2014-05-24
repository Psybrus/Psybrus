/**************************************************************************
*
* File:		ScnShaderFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNSHADERFILEDATA__
#define __SCNSHADERFILEDATA__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnShaderPermutationFlags
enum ScnShaderPermutationFlags
{
	// Render type.
	scnSPF_RENDER_FORWARD				= 0x00000001,		// Forward rendering.
	scnSPF_RENDER_DEFERRED				= 0x00000002,		// Deferred rendering.
	scnSPF_RENDER_FORWARD_PLUS			= 0x00000004,		// Forward plus rendering.
	scnSPF_RENDER_POST_PROCESS			= 0x00000008,		// Post process rendering.

	// Mesh type.
	scnSPF_MESH_STATIC_2D				= 0x00000100,		// Static 2D.
	scnSPF_MESH_STATIC_3D				= 0x00000200,		// Static 3D.
	scnSPF_MESH_SKINNED_3D				= 0x00000400,		// Skinned 3D.
	scnSPF_MESH_PARTICLE_3D				= 0x00000800,		// Particle 3D.
	scnSPF_MESH_INSTANCED_3D			= 0x00001000,		// Instanced 3D.

	// Lighting type.
	scnSPF_LIGHTING_NONE				= 0x00010000,		// Unlit geometry.
	scnSPF_LIGHTING_DIFFUSE				= 0x00020000,		// Diffuse lit geometry.
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderCodeType
enum ScnShaderCodeType
{
	scnSCT_INVALID = 0,

	// D3D11
	scnSCT_D3D11_4_0_level_9_1,
	scnSCT_D3D11_4_0_level_9_3,
	scnSCT_D3D11_4_0,
	scnSCT_D3D11_4_1,
	scnSCT_D3D11_5_1,

	// GLSL ES
	scnSCT_GLSL_ES_100,
	scnSCT_GLSL_ES_300,
	scnSCT_GLSL_ES_310,

	// GLSL
	scnSCT_GLSL_120,
	scnSCT_GLSL_130,
	scnSCT_GLSL_140,
	scnSCT_GLSL_150,
	scnSCT_GLSL_330,
	scnSCT_GLSL_400,
	scnSCT_GLSL_410,
	scnSCT_GLSL_420,
	scnSCT_GLSL_430,
	scnSCT_GLSL_440,

	scnSCT_MAX
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderHeader
struct ScnShaderHeader
{
	BcU32							NoofShaderPermutations_;
	BcU32							NoofProgramPermutations_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnShaderUnitHeader
struct ScnShaderUnitHeader
{
	RsShaderType					ShaderType_;
	RsShaderDataType				ShaderDataType_;
	ScnShaderCodeType				ShaderCodeType_;
	BcU32							ShaderHash_;
	BcU32							PermutationFlags_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderProgramHeader
struct ScnShaderProgramHeader
{
	BcU32							ProgramPermutationFlags_;
	BcU32							ShaderFlags_;
	ScnShaderCodeType				ShaderCodeType_;
	BcU32							NoofVertexAttributes_;
	BcU32							ShaderHashes_[ RsShaderType::MAX ];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
struct ScnShaderViewUniformBlockData
{
	MaMat4d							InverseProjectionTransform_;
	MaMat4d							ProjectionTransform_;
	MaMat4d							InverseViewTransform_;
	MaMat4d							ViewTransform_;
	MaMat4d							ClipTransform_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderLightUniformBlockData
struct ScnShaderLightUniformBlockData
{
	MaVec3d							LightPosition_[4];
	MaVec3d							LightDirection_[4];
	MaVec4d							LightAmbientColour_[4];
	MaVec4d							LightDiffuseColour_[4];
	MaVec3d							LightAttn_[4];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderObjectUniformBlockData
struct ScnShaderObjectUniformBlockData
{
	MaMat4d							WorldTransform_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
struct ScnShaderBoneUniformBlockData
{
	MaMat4d							BoneTransform_[24];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
struct ScnShaderAlphaTestUniformBlockData
{
	MaVec4d							AlphaTestParams_; // x = smoothstep min, y = smoothstep max, z = ref (<)
};

#endif

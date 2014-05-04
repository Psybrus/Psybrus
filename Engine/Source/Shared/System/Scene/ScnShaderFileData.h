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
	// Exclusive base implementations.
	scnSPF_STATIC_2D				= 0x00000001,		// Static 2D.
	scnSPF_STATIC_3D				= 0x00000002,		// Static 3D.
	scnSPF_SKINNED_3D				= 0x00000004,		// Skinned 3D.
	scnSPF_PARTICLE_3D				= 0x00000008,		// Particle 3D.
	scnSPF_INSTANCED_3D				= 0x00000010,		// Instanced 3D.
	scnSPF_EXCLUSIVE_BASE = scnSPF_STATIC_2D | scnSPF_STATIC_3D | scnSPF_SKINNED_3D | scnSPF_PARTICLE_3D | scnSPF_INSTANCED_3D,

	// Lighting.
	scnSPF_UNLIT					= 0x00000010,		// Unlit geometry.
	scnSPF_DIFFUSE_LIT				= 0x00000020,		// Diffuse lit geometry.
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderPermutationBootstrap
struct ScnShaderPermutationBootstrap
{
	BcU32							PermutationFlags_;
	const BcChar*					SourceUniformIncludeName_;
	const BcChar*					SourceVertexShaderName_;
	const BcChar*					SourceFragmentShaderName_;
	const BcChar*					SourceGeometryShaderName_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderHeader
struct ScnShaderHeader
{
	BcU32							NoofVertexShaderPermutations_;
	BcU32							NoofFragmentShaderPermutations_;
	BcU32							NoofGeometryShaderPermutations_;
	BcU32							NoofTessControlShaderPermutations_;
	BcU32							NoofTessEvaluationShaderPermutations_;
	BcU32							NoofProgramPermutations_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnShaderUnitHeader
struct ScnShaderUnitHeader
{
	eRsShaderType					ShaderType_;
	eRsShaderDataType				ShaderDataType_;
	BcU32							PermutationFlags_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnShaderProgramHeader
struct ScnShaderProgramHeader
{
	BcU32							ProgramPermutationFlags_;
	BcU32							ShaderFlags_;
	BcU32							NoofVertexAttributes_;
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
	MaVec2d							AlphaTestStep_;
};

#endif

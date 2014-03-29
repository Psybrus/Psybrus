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
	scnSPF_EXCLUSIVE_BASE = scnSPF_STATIC_2D | scnSPF_STATIC_3D | scnSPF_SKINNED_3D | scnSPF_PARTICLE_3D,

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
	const BcChar*					SourceGeometryShaderName_;
	const BcChar*					SourceVertexShaderName_;
	const BcChar*					SourceFragmentShaderName_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderHeader
struct ScnShaderHeader
{
	BcU32							NoofVertexShaderPermutations_;
	BcU32							NoofFragmentShaderPermutations_;
	BcU32							NoofProgramPermutations_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnShaderUnitHeader
struct ScnShaderUnitHeader
{
	BcU32							PermutationFlags_;
};
	
//////////////////////////////////////////////////////////////////////////
// ScnShaderProgramHeader
struct ScnShaderProgramHeader
{
	BcU32							ProgramPermutationFlags_;
	BcU32							VertexShaderPermutationFlags_;
	BcU32							FragmentShaderPermutationFlags_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
struct ScnShaderViewUniformBlockData
{
	BcMat4d							ClipTransform_;
	BcMat4d							ViewTransform_;
	BcMat4d							InverseViewTransform_;
	BcMat4d							WorldTransform_;
	BcVec3d							EyePosition_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderLightUniformBlockData
struct ScnShaderLightUniformBlockData
{
	BcVec3d							LightPosition_[4];
	BcVec3d							LightDirection_[4];
	BcVec4d							LightAmbientColour_[4];
	BcVec4d							LightDiffuseColour_[4];
	BcVec3d							LightAttn_[4];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
struct ScnShaderBoneUniformBlockData
{
	BcMat4d							BoneTransform_[24];
};

#endif

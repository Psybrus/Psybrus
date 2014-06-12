/**************************************************************************
*
* File:		ScnShaderFileData.h
* Author:	Neil Richardson 
* Ver/Date:	
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

#include "Reflection/ReReflection.h"

#include "System/Renderer/RsTypes.h"
#include "Math/MaMat4d.h"

//////////////////////////////////////////////////////////////////////////
// ScnShaderPermutationType
enum class ScnShaderPermutationType : BcU32
{
	// Render types.
	RENDER_FIRST,
	RENDER_FORWARD = RENDER_FIRST,				// Forward rendering.
	RENDER_DEFERRED,							// Deferred rendering.
	RENDER_FORWARD_PLUS,						// Forward plus rendering.
	RENDER_POST_PROCESS,						// Post process rendering.
	RENDER_MAX,
	RENDER_COUNT = RENDER_MAX - RENDER_FIRST,
	
	// Pass types.
	PASS_FIRST = RENDER_MAX,
	PASS_MAIN = PASS_FIRST,						// Main pass. (Typical default)
	PASS_SHADOW,								// Shadow pass (Render to shadow buffer)
	PASS_MAX,
	PASS_COUNT = PASS_MAX - PASS_FIRST,
	
	// Mesh types.
	MESH_FIRST = PASS_MAX,
	MESH_STATIC_2D = MESH_FIRST,				// Static 2D.
	MESH_STATIC_3D,								// Static 3D.
	MESH_SKINNED_3D	,							// Skinned 3D.
	MESH_PARTICLE_3D,							// Particle 3D.
	MESH_INSTANCED_3D,							// Instanced 3D.
	MESH_MAX,
	MESH_COUNT = MESH_MAX - MESH_FIRST,
	
	// Lighting types.
	LIGHTING_FIRST = MESH_MAX,
	LIGHTING_NONE = LIGHTING_FIRST,				// Unlit geometry.
	LIGHTING_DIFFUSE,							// Diffuse lit geometry.
	LIGHTING_MAX,
	LIGHTING_COUNT = LIGHTING_MAX - LIGHTING_FIRST,
};


//////////////////////////////////////////////////////////////////////////
// ScnShaderPermutationFlags
enum class ScnShaderPermutationFlags : BcU32
{
	NONE = 0,

	// Render type.
	RENDER_FORWARD				= 1 << (BcU32)ScnShaderPermutationType::RENDER_FORWARD,
	RENDER_DEFERRED				= 1 << (BcU32)ScnShaderPermutationType::RENDER_DEFERRED,
	RENDER_FORWARD_PLUS			= 1 << (BcU32)ScnShaderPermutationType::RENDER_FORWARD_PLUS,
	RENDER_POST_PROCESS			= 1 << (BcU32)ScnShaderPermutationType::RENDER_POST_PROCESS,
	RENDER_ALL = 
		RENDER_FORWARD | 
		RENDER_DEFERRED | 
		RENDER_FORWARD_PLUS | 
		RENDER_POST_PROCESS,
	
	// Pass type.
	PASS_MAIN					= 1 << (BcU32)ScnShaderPermutationType::PASS_MAIN,
	PASS_SHADOW					= 1 << (BcU32)ScnShaderPermutationType::PASS_SHADOW,
	PASS_ALL =
		PASS_MAIN |
		PASS_SHADOW,

	// Mesh type.
	MESH_STATIC_2D				= 1 << (BcU32)ScnShaderPermutationType::MESH_STATIC_2D,
	MESH_STATIC_3D				= 1 << (BcU32)ScnShaderPermutationType::MESH_STATIC_3D,
	MESH_SKINNED_3D				= 1 << (BcU32)ScnShaderPermutationType::MESH_SKINNED_3D,
	MESH_PARTICLE_3D			= 1 << (BcU32)ScnShaderPermutationType::MESH_PARTICLE_3D,
	MESH_INSTANCED_3D			= 1 << (BcU32)ScnShaderPermutationType::MESH_INSTANCED_3D,
	MESH_ALL =
		MESH_STATIC_2D |
		MESH_STATIC_3D |
		MESH_SKINNED_3D |
		MESH_PARTICLE_3D |
		MESH_INSTANCED_3D,

	// Lighting type.
	LIGHTING_NONE				= 1 << (BcU32)ScnShaderPermutationType::LIGHTING_NONE,
	LIGHTING_DIFFUSE			= 1 << (BcU32)ScnShaderPermutationType::LIGHTING_DIFFUSE,
	LIGHTING_ALL = 
		LIGHTING_NONE |
		LIGHTING_DIFFUSE,
};

inline ScnShaderPermutationFlags operator |= ( ScnShaderPermutationFlags& In, ScnShaderPermutationFlags Other )
{
	In = (ScnShaderPermutationFlags)( (int)In | (int)Other );
	return In;
}

inline ScnShaderPermutationFlags operator | ( ScnShaderPermutationFlags In, ScnShaderPermutationFlags Other )
{
	return (ScnShaderPermutationFlags)( (int)In | (int)Other );
}

inline ScnShaderPermutationFlags operator &= ( ScnShaderPermutationFlags& In, ScnShaderPermutationFlags Other )
{
	In = (ScnShaderPermutationFlags)( (int)In & (int)Other );
	return In;
}

inline ScnShaderPermutationFlags operator & ( ScnShaderPermutationFlags In, ScnShaderPermutationFlags Other )
{
	return (ScnShaderPermutationFlags)( (int)In & (int)Other );
}

//////////////////////////////////////////////////////////////////////////
// ScnShaderHeader
struct ScnShaderHeader
{
	BcU32							NoofShaderPermutations_;
	BcU32							NoofProgramPermutations_;
	BcU32							NoofShaderCodeTypes_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderUnitHeader
struct ScnShaderUnitHeader
{
	RsShaderType					ShaderType_;
	RsShaderDataType				ShaderDataType_;
	RsShaderCodeType				ShaderCodeType_;
	BcU32							ShaderHash_;
	ScnShaderPermutationFlags		PermutationFlags_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderProgramHeader
struct ScnShaderProgramHeader
{
	ScnShaderPermutationFlags		ProgramPermutationFlags_;
	ScnShaderPermutationFlags		ShaderFlags_;
	RsShaderCodeType				ShaderCodeType_;
	BcU32							NoofVertexAttributes_;
	BcU32							ShaderHashes_[ (BcU32)RsShaderType::MAX ];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
struct ScnShaderViewUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderViewUniformBlockData );
	ScnShaderViewUniformBlockData(){};

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
	REFLECTION_DECLARE_BASIC( ScnShaderLightUniformBlockData );
	ScnShaderLightUniformBlockData(){};

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
	REFLECTION_DECLARE_BASIC( ScnShaderObjectUniformBlockData );
	ScnShaderObjectUniformBlockData(){};

	MaMat4d							WorldTransform_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
struct ScnShaderBoneUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderBoneUniformBlockData );
	ScnShaderBoneUniformBlockData(){};

	static const BcU32 MAX_BONES = 24;

	MaMat4d							BoneTransform_[ MAX_BONES ];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
struct ScnShaderAlphaTestUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderAlphaTestUniformBlockData );
	ScnShaderAlphaTestUniformBlockData(){};

	MaVec4d							AlphaTestParams_; // x = smoothstep min, y = smoothstep max, z = ref (<)
};

#endif

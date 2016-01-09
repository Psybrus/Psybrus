/*******************	*******************************************************
*
* File:		Rendering/ScnShaderFileData.h
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
	RENDER_FORWARD = RENDER_FIRST,						// Forward rendering.
	RENDER_DEFERRED,									// Deferred rendering.
	RENDER_FORWARD_PLUS,								// Forward plus rendering.
	RENDER_POST_PROCESS,								// Post process rendering.
	RENDER_MAX_END,
	RENDER_COUNT = RENDER_MAX_END - RENDER_FIRST,
	
	// Pass types.
	PASS_FIRST = RENDER_MAX_END,
	PASS_SHADOW = PASS_FIRST,							// Shadow pass.
	PASS_DEPTH,											// Depth pass.
	PASS_MAIN,											// Main pass (Opaque, Transparent or Overlay passes)
	PASS_MAX_END,
	PASS_COUNT = PASS_MAX_END - PASS_FIRST,
	
	// Mesh types.
	MESH_FIRST = PASS_MAX_END,
	MESH_STATIC_2D = MESH_FIRST,						// Static 2D.
	MESH_STATIC_3D,										// Static 3D.
	MESH_SKINNED_3D	,									// Skinned 3D.
	MESH_PARTICLE_3D,									// Particle 3D.
	MESH_INSTANCED_3D,									// Instanced 3D.
	MESH_MAX_END,
	MESH_COUNT = MESH_MAX_END - MESH_FIRST,
	
	// Lighting types.
	LIGHTING_FIRST = MESH_MAX_END,
	LIGHTING_NONE = LIGHTING_FIRST,						// Unlit geometry.
	LIGHTING_DIFFUSE,									// Diffuse lit geometry.
	LIGHTING_MAX_END,
	LIGHTING_COUNT = LIGHTING_MAX_END - LIGHTING_FIRST,
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
	PASS_SHADOW					= 1 << (BcU32)ScnShaderPermutationType::PASS_SHADOW,
	PASS_DEPTH					= 1 << (BcU32)ScnShaderPermutationType::PASS_DEPTH,
	PASS_MAIN					= 1 << (BcU32)ScnShaderPermutationType::PASS_MAIN,
	PASS_ALL =
		PASS_SHADOW |
		PASS_DEPTH |
		PASS_MAIN,

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

DEFINE_ENUM_CLASS_FLAG_OPERATOR( ScnShaderPermutationFlags, | );
DEFINE_ENUM_CLASS_FLAG_OPERATOR( ScnShaderPermutationFlags, & );

//////////////////////////////////////////////////////////////////////////
// ScnShaderHeader
struct ScnShaderHeader
{
	BcU32 NoofShaderPermutations_;
	BcU32 NoofProgramPermutations_;
	BcU32 NoofShaderCodeTypes_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderUnitHeader
struct ScnShaderUnitHeader
{
	RsShaderType ShaderType_;
	RsShaderDataType ShaderDataType_;
	RsShaderCodeType ShaderCodeType_;
	BcU32 ShaderHash_;
	ScnShaderPermutationFlags PermutationFlags_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderProgramHeader
struct ScnShaderProgramHeader
{
	ScnShaderPermutationFlags ProgramPermutationFlags_;
	ScnShaderPermutationFlags ShaderFlags_;
	RsShaderCodeType ShaderCodeType_;
	BcU32 NoofVertexAttributes_;
	BcU32 NoofUniforms_;
	BcU32 NoofUniformBlocks_;
	BcU32 ShaderHashes_[ (BcU32)RsShaderType::MAX ];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
struct ScnShaderViewUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderViewUniformBlockData );
	ScnShaderViewUniformBlockData(){};

	MaMat4d InverseProjectionTransform_;
	MaMat4d ProjectionTransform_;
	MaMat4d InverseViewTransform_;
	MaMat4d ViewTransform_;
	MaMat4d InverseClipTransform_;
	MaMat4d ClipTransform_;

	/// t, t/2, t/4, t/8
	MaVec4d ViewTime_;

	/// w, h, 1/w, 1/h
	MaVec4d ViewSize_;

	/// n, f, n+f, n*f
	MaVec4d NearFar_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderLightUniformBlockData
struct ScnShaderLightUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderLightUniformBlockData );
	ScnShaderLightUniformBlockData(){};

	static const BcU32 MAX_LIGHTS = 4;

	std::array< MaVec4d, MAX_LIGHTS > LightPosition_;
	std::array< MaVec4d, MAX_LIGHTS > LightDirection_;
	std::array< MaVec4d, MAX_LIGHTS > LightAmbientColour_;
	std::array< MaVec4d, MAX_LIGHTS > LightDiffuseColour_;
	std::array< MaVec4d, MAX_LIGHTS > LightAttn_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderMaterialUniformBlockData
struct ScnShaderMaterialUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderMaterialUniformBlockData );
	ScnShaderMaterialUniformBlockData(){};

	MaVec4d MaterialBaseColour_;
	MaVec4d MaterialReflectance_;
	MaVec4d MaterialRoughness_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderObjectUniformBlockData
struct ScnShaderObjectUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderObjectUniformBlockData );
	ScnShaderObjectUniformBlockData(){};

	MaMat4d WorldTransform_;
	MaMat4d NormalTransform_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
struct ScnShaderBoneUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderBoneUniformBlockData );
	ScnShaderBoneUniformBlockData(){};

	static const BcU32 MAX_BONES = 24;

	MaMat4d BoneTransform_[ MAX_BONES ];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
struct ScnShaderAlphaTestUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderAlphaTestUniformBlockData );
	ScnShaderAlphaTestUniformBlockData(){};

	/// smoothstep min, smoothstep max, ref (<), unused
	MaVec4d AlphaTestParams_; 
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessConfigData
struct ScnShaderPostProcessConfigData
{
	REFLECTION_DECLARE_BASIC( ScnShaderPostProcessConfigData );
	ScnShaderPostProcessConfigData(){};

	/// Input texture dimensions.
	std::array< MaVec4d, 16 > InputDimensions_;
	/// Output texture dimensions.
	std::array< MaVec4d, 4 > OutputDimensions_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessCopyBlockData
struct ScnShaderPostProcessCopyBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderPostProcessCopyBlockData );
	ScnShaderPostProcessCopyBlockData(){};

	/// Colour transform to copy using.
	MaMat4d ColourTransform_;
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessBlurBlockData
struct ScnShaderPostProcessBlurBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderPostProcessBlurBlockData );
	ScnShaderPostProcessBlurBlockData(){};

	/// Texture size.
	MaVec2d TextureDimensions_;

	/// Radius in texels.
	BcF32 Radius_;

	/// Unued.
	BcF32 Unused_;
};

#endif

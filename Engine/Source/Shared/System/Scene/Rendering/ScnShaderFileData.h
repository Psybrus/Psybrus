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
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderPermutationFlags
enum class ScnShaderPermutationFlags : BcU32
{
	NONE = 0,

	// Render type.
	RENDER_FORWARD				= 1 << (BcU32)ScnShaderPermutationType::RENDER_FORWARD,
	RENDER_DEFERRED				= 1 << (BcU32)ScnShaderPermutationType::RENDER_DEFERRED,
	RENDER_POST_PROCESS			= 1 << (BcU32)ScnShaderPermutationType::RENDER_POST_PROCESS,
	RENDER_ALL = 
		RENDER_FORWARD | 
		RENDER_DEFERRED | 
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
};

DEFINE_ENUM_CLASS_FLAG_OPERATOR( ScnShaderPermutationFlags, | );
DEFINE_ENUM_CLASS_FLAG_OPERATOR( ScnShaderPermutationFlags, & );
DEFINE_ENUM_CLASS_UNARY_FLAG_OPERATOR( ScnShaderPermutationFlags, ~ );

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
	BcU32 NoofParameters_;
	BcU32 ShaderHashes_[ (BcU32)RsShaderType::MAX ];
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderDataAttribute
class ScnShaderDataAttribute:
	public ReAttribute
{
public:
	REFLECTION_DECLARE_DERIVED( ScnShaderDataAttribute, ReAttribute );

	ScnShaderDataAttribute( BcName Name = BcName::INVALID, BcBool IsInstancable = BcFalse );
	virtual ~ScnShaderDataAttribute();

	bool isInstancable() const { return !!IsInstancable_; }

private:
	BcBool IsInstancable_ = BcFalse;
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
	MaVec4d ViewTime_ = MaVec4d( 0.0f, 0.0f, 0.0f, 0.0f );

	/// w, h, 1/w, 1/h
	MaVec4d ViewSize_ = MaVec4d( 0.0f, 0.0f, 0.0f, 0.0f );

	/// n, f, n+f, n*f
	MaVec4d NearFar_ = MaVec4d( 0.0f, 0.0f, 0.0f, 0.0f );
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

	MaVec4d MaterialBaseColour_ = MaVec4d( 1.0f, 1.0f, 1.0f, 1.0f );
	BcF32 MaterialMetallic_ = 1.0f;
	BcF32 MaterialRoughness_ = 1.0f;
	BcF32 MaterialUnused0_ = 0.0f;
	BcF32 MaterialUnused1_ = 0.0f;
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
	MaVec4d AlphaTestParams_ = MaVec4d( 0.45f, 5.0f, 0.0f, 0.0f );
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
// ScnShaderDownsampleUniformBlockData
struct ScnShaderDownsampleUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderDownsampleUniformBlockData );
	ScnShaderDownsampleUniformBlockData(){};

	BcU32 DownsampleSourceMipLevel_ = 0;
	BcU32 DownsampleUnused_[ 3 ] = {};
};

//////////////////////////////////////////////////////////////////////////
// ScnShaderToneMappingUniformBlockData
struct ScnShaderToneMappingUniformBlockData
{
	REFLECTION_DECLARE_BASIC( ScnShaderToneMappingUniformBlockData );
	ScnShaderToneMappingUniformBlockData(){};

	/// Exposure.
	BcF32 ToneMappingExposure_ = 0.0f;
	/// Middle grey.
	BcF32 ToneMappingMiddleGrey_ = 1.0f;
	/// White point.
	BcF32 ToneMappingWhitePoint_ = 11.2f;
	/// Constants for curve.
	BcF32 ToneMappingA_ = 0.15f;
	BcF32 ToneMappingB_ = 0.50f;
	BcF32 ToneMappingC_ = 0.10f;
	BcF32 ToneMappingD_ = 0.20f;
	BcF32 ToneMappingE_ = 0.02f;
	BcF32 ToneMappingF_ = 0.30f;
	/// Minimum luminance.
	BcF32 ToneMappingLuminanceMin_ = 0.05f;
	/// Maximum luminance.
	BcF32 ToneMappingLuminanceMax_ = 100.0f;
	/// Luminance transfer rate.
	BcF32 ToneMappingLuminanceTransferRate_ = 0.05f;

	/// Methods for reference/utility.
	BcF32 uncharted2Curve( BcF32 X );
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
	MaVec2d TextureDimensions_ = MaVec2d( 0.0f, 0.0f );

	/// Radius in texels.
	BcF32 Radius_ = 1.0f;

	/// Unued.
	BcF32 BlurUnused_ = 0.0f;
};

#endif

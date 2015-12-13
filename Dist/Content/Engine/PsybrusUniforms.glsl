#pragma once

////////////////////////////////////////////////////////////////////////
// cbuffer macros

#if PSY_INPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES
#undef PSY_USE_CBUFFER
#define PSY_USE_CBUFFER 0
#else
#undef PSY_USE_CBUFFER
#define PSY_USE_CBUFFER 1
#endif


#if PSY_USE_CBUFFER
#  if PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_430
#    define BEGIN_CBUFFER( _n, _slot ) layout(std140) uniform _n {
#  else
#    define BEGIN_CBUFFER( _n, _slot ) layout(std140) uniform _n {
#  endif
#  define ENTRY( _p, _t, _n ) _t _n;
#  define END_CBUFFER };
#else
#  define BEGIN_CBUFFER( _n, _slot )
#  define ENTRY( _p, _t, _n ) uniform _t _p##VS_X##_n;
#  define END_CBUFFER
#endif

////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
BEGIN_CBUFFER( ScnShaderViewUniformBlockData, 0 )
	ENTRY( ScnShaderViewUniformBlockData, float4x4, InverseProjectionTransform_ )
	ENTRY( ScnShaderViewUniformBlockData, float4x4, ProjectionTransform_ )
	ENTRY( ScnShaderViewUniformBlockData, float4x4, InverseViewTransform_ )
	ENTRY( ScnShaderViewUniformBlockData, float4x4, ViewTransform_ )
	ENTRY( ScnShaderViewUniformBlockData, float4x4, ClipTransform_ )
	ENTRY( ScnShaderViewUniformBlockData, float4, ViewTime_ )
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define InverseProjectionTransform_ ScnShaderViewUniformBlockDataVS_XInverseProjectionTransform_
#  define ProjectionTransform_ ScnShaderViewUniformBlockDataVS_XProjectionTransform_
#  define InverseViewTransform_ ScnShaderViewUniformBlockDataVS_XInverseViewTransform_
#  define ViewTransform_ ScnShaderViewUniformBlockDataVS_XViewTransform_
#  define ClipTransform_ ScnShaderViewUniformBlockDataVS_XClipTransform_
#  define ViewTime_ ScnShaderViewUniformBlockDataVS_XViewTime_

#endif

////////////////////////////////////////////////////////////////////////
// ScnShaderObjectUniformBlockData
BEGIN_CBUFFER( ScnShaderObjectUniformBlockData, 1 )
	ENTRY( ScnShaderObjectUniformBlockData, float4x4, WorldTransform_ )
	ENTRY( ScnShaderObjectUniformBlockData, float4x4, NormalTransform_ )
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define WorldTransform_ ScnShaderObjectUniformBlockDataVS_XWorldTransform_
#  define NormalTransform_ ScnShaderObjectUniformBlockDataVS_XNormalTransform_

#endif


////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
BEGIN_CBUFFER( ScnShaderBoneUniformBlockData, 2 )
	ENTRY( ScnShaderBoneUniformBlockData, float4x4, BoneTransform_[24] )
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define BoneTransform_ ScnShaderBoneUniformBlockDataVS_XBoneTransform_

#endif


////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
BEGIN_CBUFFER( ScnShaderAlphaTestUniformBlockData, 3 )
	/// x = smoothstep min, y = smoothstep max, z = ref (<)
	ENTRY( ScnShaderAlphaTestUniformBlockData, float4, AlphaTestParams_ ) 
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define AlphaTestParams_ ScnShaderAlphaTestUniformBlockDataVS_XAlphaTestParams_

#endif

////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessConfigData
BEGIN_CBUFFER( ScnShaderPostProcessConfigData, 4 )
	ENTRY( ScnShaderPostProcessConfigData, float4, InputDimensions_[16] ) 
	ENTRY( ScnShaderPostProcessConfigData, float4, OutputDimensions_[4] ) 
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define InputDimensions_ ScnShaderPostProcessConfigDataVS_XInputDimensions_
#  define OutputDimensions_ ScnShaderPostProcessConfigDataVS_XOutputDimensions_

#endif

////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessCopyBlockData
BEGIN_CBUFFER( ScnShaderPostProcessCopyBlockData, 5 )
	/// Colour transform to copy using.
	ENTRY( ScnShaderPostProcessCopyBlockData, float4x4, ColourTransform_ ) 
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define ColourTransform_ ScnShaderPostProcessCopyBlockDataVS_XColourTransform_

#endif

////////////////////////////////////////////////////////////////////////
// ScnShaderPostProcessBlurBlockData
BEGIN_CBUFFER( ScnShaderPostProcessBlurBlockData, 6 )
	ENTRY( ScnShaderPostProcessBlurBlockData, float2, TextureDimensions_ ) 
	ENTRY( ScnShaderPostProcessBlurBlockData, float, Radius_ ) 
	ENTRY( ScnShaderPostProcessBlurBlockData, float, Unused_ ) 
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define TextureDimensions_ ScnShaderPostProcessBlurBlockDataVS_XTextureDimensions_
#  define Radius_ ScnShaderPostProcessBlurBlockDataVS_XRadius_

#endif

////////////////////////////////////////////////////////////////////////
// ScnFontUniformBlockData
BEGIN_CBUFFER( ScnFontUniformBlockData, 7 )
	/// x = smoothstep min, y = smoothstep max, z = ref (<)
	ENTRY( ScnFontUniformBlockData, float4, TextSettings_ ) 
	ENTRY( ScnFontUniformBlockData, float4, BorderSettings_ ) 
	ENTRY( ScnFontUniformBlockData, float4, ShadowSettings_ ) 
	ENTRY( ScnFontUniformBlockData, float4, TextColour_ ) 
	ENTRY( ScnFontUniformBlockData, float4, BorderColour_ ) 
	ENTRY( ScnFontUniformBlockData, float4, ShadowColour_ ) 
END_CBUFFER

#if !PSY_USE_CBUFFER

#  define TextSettings_ ScnFontUniformBlockDataVS_XTextSettings_
#  define BorderSettings_ ScnFontUniformBlockDataVS_XBorderSettings_
#  define ShadowSettings_ ScnFontUniformBlockDataVS_XShadowSettings_
#  define TextColour_ ScnFontUniformBlockDataVS_XTextColour_
#  define BorderColour_ ScnFontUniformBlockDataVS_XBorderColour_
#  define ShadowColour_ ScnFontUniformBlockDataVS_XShadowColour_

#endif

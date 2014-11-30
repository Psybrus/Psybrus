////////////////////////////////////////////////////////////////////////
// cbuffer macros
#if PSY_USE_CBUFFER
#	define BEGIN_CBUFFER( _n ) cbuffer _n {
#	define ENTRY( _t, _n ) _t _n;
#	define END_CBUFFER };
#else
#	define BEGIN_CBUFFER( _n ) struct _n {
#	define ENTRY( _t, _n ) _t X##_n;
#	define END_CBUFFER };
#endif



////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
BEGIN_CBUFFER( ScnShaderViewUniformBlockData )
	ENTRY( float4x4, InverseProjectionTransform_ )
	ENTRY( float4x4, ProjectionTransform_ )
	ENTRY( float4x4, InverseViewTransform_ )
	ENTRY( float4x4, ViewTransform_ )
	ENTRY( float4x4, ClipTransform_ )
END_CBUFFER

#if !PSY_USE_CBUFFER
ScnShaderViewUniformBlockData ScnShaderViewUniformBlockDataVS;

#  define InverseProjectionTransform_ ScnShaderViewUniformBlockDataVS.XInverseProjectionTransform_
#  define ProjectionTransform_ ScnShaderViewUniformBlockDataVS.XProjectionTransform_
#  define InverseViewTransform_ ScnShaderViewUniformBlockDataVS.XInverseViewTransform_
#  define ViewTransform_ ScnShaderViewUniformBlockDataVS.XViewTransform_
#  define ClipTransform_ ScnShaderViewUniformBlockDataVS.XClipTransform_

#endif


////////////////////////////////////////////////////////////////////////
// ScnShaderLightUniformBlockData
BEGIN_CBUFFER( ScnShaderLightUniformBlockData )
	ENTRY( float3, LightPosition_[4] )
	ENTRY( float3, LightDirection_[4] )
	ENTRY( float4, LightAmbientColour_[4] )
	ENTRY( float4, LightDiffuseColour_[4] )
	ENTRY( float3, LightAttn_[4] )
END_CBUFFER

#if !PSY_USE_CBUFFER
ScnShaderLightUniformBlockData ScnShaderLightUniformBlockDataVS;

#  define LightPosition_ ScnShaderLightUniformBlockDataVS.XLightPosition_
#  define LightDirection_ ScnShaderLightUniformBlockDataVS.XLightDirection_
#  define LightAmbientColour_ ScnShaderLightUniformBlockDataVS.XLightAmbientColour_
#  define LightDiffuseColour_ ScnShaderLightUniformBlockDataVS.XLightDiffuseColour_
#  define LightAttn_ ScnShaderLightUniformBlockDataVS.XLightAttn_

#endif


////////////////////////////////////////////////////////////////////////
// ScnShaderObjectUniformBlockData
BEGIN_CBUFFER( ScnShaderObjectUniformBlockData )
	ENTRY( float4x4, WorldTransform_ )
	ENTRY( float4x4, NormalTransform_ )
END_CBUFFER

#if !PSY_USE_CBUFFER
ScnShaderObjectUniformBlockData ScnShaderObjectUniformBlockDataVS;

#  define WorldTransform_ ScnShaderObjectUniformBlockDataVS.XWorldTransform_
#  define NormalTransform_ ScnShaderObjectUniformBlockDataVS.XNormalTransform_

#endif


////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
BEGIN_CBUFFER( ScnShaderBoneUniformBlockData )
	ENTRY( float4, BoneTransform_[24*4] )
END_CBUFFER

#if !PSY_USE_CBUFFER
ScnShaderBoneUniformBlockData ScnShaderBoneUniformBlockDataVS;

#  define BoneTransform_ ScnShaderBoneUniformBlockDataVS.XBoneTransform_

#endif


////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
BEGIN_CBUFFER( ScnShaderAlphaTestUniformBlockData )
	/// x = smoothstep min, y = smoothstep max, z = ref (<)
	ENTRY( float4, AlphaTestParams_ ) 
END_CBUFFER

#if !PSY_USE_CBUFFER
ScnShaderAlphaTestUniformBlockData ScnShaderAlphaTestUniformBlockDataVS;

#  define AlphaTestParams_ ScnShaderAlphaTestUniformBlockDataVS.XAlphaTestParams_

#endif


////////////////////////////////////////////////////////////////////////
// Get bone transform vector
float4 PsyGetBoneTransformVector( const float i, const int e )
{
#if PSY_BACKEND_TYPE != PSY_BACKEND_TYPE_GLSL_ES
	return BoneTransform_[ int( i ) + e ];
#else
	return float4( 0.0f, 0.0f, 0.0f, 0.0f );
#endif
}


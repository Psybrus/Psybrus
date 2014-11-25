////////////////////////////////////////////////////////////////////////
// cbuffer macros
#if PSY_USE_CBUFFER
#	define BEGIN_CBUFFER( _n )	cbuffer _n {
#	define END_CBUFFER };
#else
#	define BEGIN_CBUFFER( _n )
#	define END_CBUFFER
#endif

////////////////////////////////////////////////////////////////////////
// ScnShaderViewUniformBlockData
BEGIN_CBUFFER( ScnShaderViewUniformBlockData )
	float4x4 InverseProjectionTransform_;
	float4x4 ProjectionTransform_;
	float4x4 InverseViewTransform_;
	float4x4 ViewTransform_;
	float4x4 ClipTransform_;
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// ScnShaderLightUniformBlockData
BEGIN_CBUFFER( ScnShaderLightUniformBlockData )
	float3 LightPosition_[4];
	float3 LightDirection_[4];
	float4 LightAmbientColour_[4];
	float4 LightDiffuseColour_[4];
	float3 LightAttn_[4];
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// ScnShaderObjectUniformBlockData
BEGIN_CBUFFER( ScnShaderObjectUniformBlockData )
	float4x4 WorldTransform_;
	float4x4 NormalTransform_;
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// ScnShaderBoneUniformBlockData
BEGIN_CBUFFER( ScnShaderBoneUniformBlockData )
	float4 BoneTransform_[24*4];
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// ScnShaderAlphaTestUniformBlockData
BEGIN_CBUFFER( ScnShaderAlphaTestUniformBlockData )
	float4 AlphaTestParams_; // x = smoothstep min, y = smoothstep max, z = ref (<)
END_CBUFFER


////////////////////////////////////////////////////////////////////////
// Get bone transform vector
float4 PsyGetBoneTransformVector( const float i, const int e )
{
	return BoneTransform_[ int( i ) + e ];
}


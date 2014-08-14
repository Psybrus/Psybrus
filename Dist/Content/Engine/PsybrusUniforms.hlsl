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
// ViewUniformBlock
BEGIN_CBUFFER( ViewUniformBlock )
	float4x4 InverseProjectionTransform_;
	float4x4 ProjectionTransform_;
	float4x4 InverseViewTransform_;
	float4x4 ViewTransform_;
	float4x4 ClipTransform_;
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// LightUniformBlock
BEGIN_CBUFFER( LightUniformBlock )
	float3 LightPosition_[4];
	float3 LightDirection_[4];
	float4 LightAmbientColour_[4];
	float4 LightDiffuseColour_[4];
	float3 LightAttn_[4];
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// ObjectUniformBlock
BEGIN_CBUFFER( ObjectUniformBlock )
	float4x4 WorldTransform_;
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// BoneUniformBlock
BEGIN_CBUFFER( BoneUniformBlock )
	float4x4 BoneTransform_[24];
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// AlphaTestUniformBlock
BEGIN_CBUFFER( AlphaTestUniformBlock )
	float3 AlphaTestParams_; // x = smoothstep min, y = smoothstep max, z = ref (<)
END_CBUFFER

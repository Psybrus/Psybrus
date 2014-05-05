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
	float4x4 uInverseProjectionTransform;
	float4x4 uProjectionTransform;
	float4x4 uInverseViewTransform;
	float4x4 uViewTransform;
	float4x4 uClipTransform;
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// LightUniformBlock
BEGIN_CBUFFER( LightUniformBlock )
	float3 uLightPosition[4];
	float3 uLightDirection[4];
	float4 uLightAmbientColour[4];
	float4 uLightDiffuseColour[4];
	float3 uLightAttn[4];
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// ObjectUniformBlock
BEGIN_CBUFFER( ObjectUniformBlock )
	float4x4 uWorldTransform;
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// BoneUniformBlock
BEGIN_CBUFFER( BoneUniformBlock )
	float4x4 uBoneTransform[24];
END_CBUFFER

////////////////////////////////////////////////////////////////////////
// AlphaTestUniformBlock
BEGIN_CBUFFER( AlphaTestUniformBlock )
	float3 uAlphaTestParams; // x = smoothstep min, y = smoothstep max, z = ref (<)
END_CBUFFER

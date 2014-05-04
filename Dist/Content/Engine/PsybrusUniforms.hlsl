cbuffer ViewUniformBlock
{
	float4x4 uInverseProjectionTransform;
	float4x4 uProjectionTransform;
	float4x4 uInverseViewTransform;
	float4x4 uViewTransform;
	float4x4 uClipTransform;
};

cbuffer LightUniformBlock
{
	float3 uLightPosition[4];
	float3 uLightDirection[4];
	float4 uLightAmbientColour[4];
	float4 uLightDiffuseColour[4];
	float3 uLightAttn[4];
};

cbuffer ObjectUniformBlock
{
	float4x4 uWorldTransform;
};

cbuffer BoneUniformBlock
{
	float4x4 uBoneTransform[24];
};

cbuffer AlphaTestUniformBlock
{
	float2 aAlphaTestStep;
};

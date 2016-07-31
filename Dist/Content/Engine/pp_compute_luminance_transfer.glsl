#pragma version 430 core
#define SKIP_VERSION

#include <PsybrusTypes.psh>
#include <PsybrusUtility.psh>
#include <UniformToneMapping.psh>

precision highp float;
precision highp int;

layout (local_size_x = 1, local_size_y = 1) in;

layout(r32f) readonly uniform image2D aInputTexture;
layout(r32f) uniform image2D aOutputTexture;

void main()
{
	int2 Id = int2( gl_GlobalInvocationID.xy );
	float TexelI = imageLoad( aInputTexture, Id ).x;
	float TexelO = imageLoad( aOutputTexture, Id ).x;
	float Luminance = clamp( TexelO + ( TexelI - TexelO ) * ToneMappingLuminanceTransferRate_, ToneMappingLuminanceMin_, ToneMappingLuminanceMax_ );
	imageStore( aOutputTexture, Id, float4( Luminance, 0.0, 0.0, 0.0 ) );
}

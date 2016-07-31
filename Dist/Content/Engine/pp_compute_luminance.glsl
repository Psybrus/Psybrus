#pragma version 430 core
#define SKIP_VERSION

#include <PsybrusTypes.psh>
#include <PsybrusUtility.psh>

precision highp float;
precision highp int;

layout (local_size_x = 1, local_size_y = 1) in;

layout(rgba16f) readonly uniform image2D aInputTexture;
layout(r32f) writeonly uniform image2D aOutputTexture;

void main()
{
	int2 iId = int2( gl_GlobalInvocationID.xy ) * 2;
	int2 oId = int2( gl_GlobalInvocationID.xy );
	int2 inputSize = imageSize( aInputTexture );
	iId.x = clamp( iId.x, 0, max( 0, inputSize.x - 2 ) );
	iId.y = clamp( iId.y, 0, max( 0, inputSize.y - 2 ) );
	float3 texel = 
		imageLoad( aInputTexture, iId + int2( 0, 0 ) ).xyz +
		imageLoad( aInputTexture, iId + int2( 1, 0 ) ).xyz +
		imageLoad( aInputTexture, iId + int2( 0, 1 ) ).xyz +
		imageLoad( aInputTexture, iId + int2( 1, 1 ) ).xyz;	
	imageStore( aOutputTexture, oId, float4( luminance( texel / 4.0 ), 0.0, 0.0, 0.0 ) );
}

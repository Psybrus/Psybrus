#pragma version 430 core
#define SKIP_VERSION

#include "pp_shared.psh"

#include <PsybrusLighting.psh>
#include <PsybrusSamplers.psh>
#include <PsybrusUtility.psh>

#include <UniformEnvironmentFilter.psh>

precision highp float;
precision highp int;

layout (local_size_x = 1, local_size_y = 1) in;

PSY_SAMPLER_CUBE( InputTexture );

#if OUTPUT_RGBA8
#  define DECODE_COLOUR gammaToLinear
#  define ENCODE_COLOUR linearToGamma

layout(rgba8) writeonly uniform image2D aOutputTexture;

#elif OUTPUT_RGBA16F
#  define DECODE_COLOUR
#  define ENCODE_COLOUR

layout(rgba16f) writeonly uniform image2D aOutputTexture;

#endif

void main()
{
	int2 iId = int2( gl_GlobalInvocationID.xy );
	int2 oId = int2( gl_GlobalInvocationID.xy );
	int2 TextureSize = imageSize( aOutputTexture );
	float2 TextureHalfSize = float2( TextureSize ) * 0.5;
	float2 Coord = ( ( float2( iId ) + float2( 0.5, 0.5 ) ) - TextureHalfSize ) / TextureHalfSize;

	float3 N = getCubemapNormal( EnvironmentFilterCubeFace_, Coord );
	float3 Total = float3( 0.0, 0.0, 0.0 );
	float Roughness = EnvironmentFilterRoughness_;
	int NumSamples = 1024;
	for( int Idx = 0; Idx < NumSamples; ++Idx )
	{
		float3 H = importanceSampleGGX( hammersley( Idx, NumSamples ), Roughness, N );
		float3 L = 2 * dot( N, H ) * H - N;
		float NdotL = clamp( dot( N, L ), 0.0, 1.0 );
		if( NdotL > 0.0 )
		{
			float4 SampleColour = DECODE_COLOUR( PSY_SAMPLE_CUBE( InputTexture, L ) );
			Total += SampleColour.xyz * NdotL;
		}
	}

	Total /= float( NumSamples );
	imageStore( aOutputTexture, oId, ENCODE_COLOUR( float4( Total, 0.0 ) ) );
}

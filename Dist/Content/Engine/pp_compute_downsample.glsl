#pragma version 430 core
#define SKIP_VERSION

#include <PsybrusTypes.psh>
#include <PsybrusUtility.psh>

precision highp float;
precision highp int;

layout (local_size_x = 1, local_size_y = 1) in;

#if OUTPUT_RGBA8
#  define COLOUR_TYPE float4 
#  define CHANNELS xyzw
#  define DECODE_COLOUR( a ) gammaToLinear( a )
#  define ENCODE_COLOUR( a ) linearToGamma( a )

layout(rgba8) readonly uniform image2D aInputTexture;
layout(rgba8) writeonly uniform image2D aOutputTexture;

#elif OUTPUT_RGBA16F
#  define COLOUR_TYPE float4 
#  define CHANNELS xyzw
#  define DECODE_COLOUR( a ) a
#  define ENCODE_COLOUR( a ) a

layout(rgba16f) readonly uniform image2D aInputTexture;
layout(rgba16f) writeonly uniform image2D aOutputTexture;

#elif OUTPUT_R16F
#  define COLOUR_TYPE float 
#  define CHANNELS x
#  define DECODE_COLOUR( a ) a
#  define ENCODE_COLOUR( a ) float4( a, 0.0, 0.0, 0.0 )

layout(r16f) readonly uniform image2D aInputTexture;
layout(r16f) writeonly uniform image2D aOutputTexture;

#elif OUTPUT_R32F
#  define COLOUR_TYPE float 
#  define CHANNELS x
#  define DECODE_COLOUR( a ) a
#  define ENCODE_COLOUR( a ) float4( a, 0.0, 0.0, 0.0 )

layout(r32f) readonly uniform image2D aInputTexture;
layout(r32f) writeonly uniform image2D aOutputTexture;
#endif

void main()
{
	int2 iId = int2(gl_GlobalInvocationID.xy) * 2;
	int2 oId = int2(gl_GlobalInvocationID.xy);
	int2 inputSize = imageSize( aInputTexture );
	iId.x = clamp( iId.x, 0, inputSize.x - 2 );
	iId.y = clamp( iId.y, 0, inputSize.y - 2 );
	COLOUR_TYPE texel = 
		DECODE_COLOUR( imageLoad( aInputTexture, iId + int2( 0, 0 ) ).CHANNELS ) +
		DECODE_COLOUR( imageLoad( aInputTexture, iId + int2( 1, 0 ) ).CHANNELS ) +
		DECODE_COLOUR( imageLoad( aInputTexture, iId + int2( 0, 1 ) ).CHANNELS ) + 
		DECODE_COLOUR( imageLoad( aInputTexture, iId + int2( 1, 1 ) ).CHANNELS );
	imageStore( aOutputTexture, oId, ENCODE_COLOUR( texel / 4.0 ) );
}

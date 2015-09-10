#include <Psybrus.glsl>

//////////////////////////////////////////////////////////////////////////
// Vertex shader
#if VERTEX_SHADER

VS_IN( vec4, InPosition_, POSITION );
VS_IN( vec4, InTexCoord_, TEXCOORD0 );
VS_IN( vec4, InColour_, COLOUR0 );

VS_OUT( vec4, VsColour0 );
VS_OUT( vec4, VsTexCoord0 );

void vertexMain()
{
    gl_Position = mul( ClipTransform_, float4( InPosition_.xy, 0.0, 1.0 ) );
    VsTexCoord0 = InTexCoord_;
    VsColour0 = InColour_;
}

#endif

//////////////////////////////////////////////////////////////////////////
// Pixel shader
#if PIXEL_SHADER

PS_IN( vec4, VsColour0 );
PS_IN( vec4, VsTexCoord0 );

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_330
out float4 fragColor;
#endif

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#define fragColor gl_FragData[0]
#endif

//////////////////////////////////////////////////////////////////////////
// pixelDefaultMain
void pixelDefaultMain()
{
	fragColor = VsColour0;
}

//////////////////////////////////////////////////////////////////////////
// pixelTexturedMain
PSY_SAMPLER_2D( DiffuseTex );

void pixelTexturedMain()
{
	vec4 Colour = PSY_SAMPLE_2D( DiffuseTex, VsTexCoord0.xy );
	fragColor = Colour * VsColour0;
}

#endif

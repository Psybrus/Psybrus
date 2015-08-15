#include <Psybrus.glsl>

//////////////////////////////////////////////////////////////////////////
// Vertex shader
#if VERTEX_SHADER

VS_IN( vec4, InPosition_, POSITION );
VS_IN( vec4, InNormal_, NORMAL );
VS_IN( vec4, InTexCoord_, TEXCOORD0 );
VS_IN( vec4, InColour_, COLOUR0 );

#if defined( PERM_MESH_SKINNED_3D )

VS_IN( vec4, InBlendWeights_, BLENDWEIGHTS );
VS_IN( vec4, InBlendIndices_, BLENDINDICES );

#elif defined( PERM_MESH_PARTICLE_3D )

VS_IN( vec4, InVertexOffset_, TANGENT );

#endif

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#define VsColour0 GsColour0
#define VsNormal GsNormal
#define VsTexCoord0 GsTexCoord0
#endif

VS_OUT( vec4, VsColour0 );
VS_OUT( vec4, VsNormal );
VS_OUT( vec4, VsTexCoord0 );

void vertexMain()
{
 	vec4 WorldPosition;
	PSY_MAKE_WORLD_SPACE_VERTEX( WorldPosition, InPosition_ );
	PSY_MAKE_CLIP_SPACE_VERTEX( gl_Position, WorldPosition );
	VsNormal = InNormal_;
	VsTexCoord0 = InTexCoord_;
	VsColour0 = InColour_;
}

#endif

//////////////////////////////////////////////////////////////////////////
// Geometry shader
#if GEOMETRY_SHADER

GS_IN( vec4, VsColour0 );
GS_IN( vec4, VsNormal );
GS_IN( vec4, VsTexCoord0 );

GS_OUT( vec4, GsColour0 );
GS_OUT( vec4, GsNormal );
GS_OUT( vec4, GsTexCoord0 );

void geometryMain()
{
	for( float j = -4; j <= 4; j += 1.0 )
	{
		for( int i = 0; i < 3; ++i )
		{
			float4 WorldPosition;
			PSY_MAKE_WORLD_SPACE_VERTEX( WorldPosition, Input[i].Position_, Input[i] );

			WorldPosition.z += j * 10.0;

			PSY_MAKE_CLIP_SPACE_VERTEX( gl_Position, WorldPosition );

			Output.Normal_ = Input[i].Normal_;
			GsColour0 = Input[i].Colour_;
			GsTexCoord0 = Input[i].TexCoord0_;

			EmitVertex();
		}
		EmitPrimitive();
	}
}

#endif

//////////////////////////////////////////////////////////////////////////
// Pixel shader
#if PIXEL_SHADER

PS_IN( vec4, GsColour0 );
PS_IN( vec4, GsNormal );
PS_IN( vec4, GsTexCoord0 );

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_330
out float4 fragColor;
#endif

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#define fragColor gl_FragData[0]
#endif

//////////////////////////////////////////////////////////////////////////
// pixelMain
void pixelMain()
{
	fragColor.rgb = ( VsNormal.rgb + vec3( 1.0f, 1.0f, 1.0f ) ) * 0.5f;
	fragColor.w = 1.0f;
}

#endif

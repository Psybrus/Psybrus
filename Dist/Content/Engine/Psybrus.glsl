#pragma once

//////////////////////////////////////////////////////////////////////////
// Version directives.
#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_150
#version 150

#elif PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_330
#version 330

#elif PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100

#version 100
#extension GL_EXT_draw_buffers : enable
precision mediump float;

#else
#error Invalid output code type - PSY_OUTPUT_CODE_TYPE
#endif

//////////////////////////////////////////////////////////////////////////
// HLSL types for easy sharing.
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float3x3 mat3
#define float4x4 mat4

//////////////////////////////////////////////////////////////////////////
// PI
#define PI (3.14159265358979310)
#define PIMUL2 (6.28318530717958620)
#define PIMUL4 (12.5663706143591720)
#define PIDIV2 (1.57079632679489660)
#define PIDIV4 (0.78539816339744828)


//////////////////////////////////////////////////////////////////////////
// Input attributes.
#define VsPosition gl_Position

#if ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_140 ) || ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_ES_300 )
#  define VS_IN( _Ty, _Name, _Semantic )		in _Ty _Name : _Semantic
#  define VS_OUT( _Ty, _Name )					out _Ty _Name

#  define GS_IN( _Ty, _Name )					in _Ty _Name
#  define GS_OUT( _Ty, _Name )					out _Ty _Name

#  define PS_IN( _Ty, _Name )					in _Ty _Name

#else
#  define VS_IN( _Ty, _Name, _Semantic ) 		attribute _Ty _Name : _Semantic
#  define VS_OUT( _Ty, _Name )					varying _Ty _Name

#  define GS_IN( _Ty, _Name )					in _Ty _Name
#  define GS_OUT( _Ty, _Name )					out _Ty _Name

#  define PS_IN( _Ty, _Name )					varying _Ty _Name

#endif

//////////////////////////////////////////////////////////////////////////
// Transpose
#if ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL && PSY_OUTPUT_CODE_TYPE < PSY_CODE_TYPE_GLSL_150 ) || ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES && PSY_OUTPUT_CODE_TYPE < PSY_CODE_TYPE_GLSL_ES_300 )
mat3 transpose( mat3 In )
{
	return mat3( 
		vec3( In[0].x, In[1].x, In[2].x ),
		vec3( In[0].y, In[1].y, In[2].y ),
		vec3( In[0].z, In[1].z, In[2].z ) );
}

mat4 transpose( mat4 In )
{
	return mat4( 
		vec4( In[0].x, In[1].x, In[2].x, In[3].x ),
		vec4( In[0].y, In[1].y, In[2].y, In[3].y ),
		vec4( In[0].z, In[1].z, In[2].z, In[3].z ),
		vec4( In[0].w, In[1].w, In[2].w, In[3].w ) );
}
#endif

//////////////////////////////////////////////////////////////////////////
// HLSL mul.
vec3 mul( mat3 M, vec3 V )
{
	return M * V;
}

vec4 mul( mat4 M, vec4 V )
{
	return M * V;
}

//////////////////////////////////////////////////////////////////////////
// Colour space conversion.
#define FAST_GAMMA_CONVERSION ( 0 )

vec3 gammaToLinear( vec3 InputRGBA )
{
#if FAST_GAMMA_CONVERSION
	return vec3( InputRGBA.rgb * InputRGBA.rgb );
#else
	return vec3( pow( InputRGBA.rgb, vec3( 2.2 ) ) ); 
#endif
}

vec4 gammaToLinear( vec4 InputRGBA )
{
	return vec4( gammaToLinear( InputRGBA.rgb ), InputRGBA.a );
}

vec3 linearToGamma( vec3 InputRGBA )
{
#if FAST_GAMMA_CONVERSION
	return vec3( sqrt( InputRGBA.rgb ) );
#else	
	return vec3( pow( InputRGBA.rgb, vec3( 1.0 / 2.2 ) ) );
#endif
}

vec4 linearToGamma( vec4 InputRGBA )
{
	return vec4( linearToGamma( InputRGBA.rgb ), InputRGBA.a );
}

//////////////////////////////////////////////////////////////////////////
// Depth utility.
float linearDepth( float DepthSample, float Near, float Far )
{
	DepthSample = 2.0 * DepthSample - 1.0;
	return ( 2.0 * ( Near * Far ) ) / ( ( Near + Far ) - DepthSample * ( Far - Near ) );
}

//////////////////////////////////////////////////////////////////////////
// PSY_MAKE_WORLD_SPACE_VERTEX
#if defined( PERM_MESH_STATIC_2D )
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) \
	_o = vec4( _v.xy, 0.0, 1.0 );
#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v ) \
	_o = vec4( _v.xy, 0.0, 1.0 );

#elif defined( PERM_MESH_STATIC_3D )

#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) \
	_o = mul( WorldTransform_, _v );
#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v ) \
	_o = mul( NormalTransform_, _v ); \
	_o.xyz = normalize( _o.xyz );

#elif defined( PERM_MESH_SKINNED_3D )

#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) \
	_o =  mul( BoneTransform_[ int(SEMANTIC_BLENDINDICES.x) ], _v ) * SEMANTIC_BLENDWEIGHTS.x; \
	_o += mul( BoneTransform_[ int(SEMANTIC_BLENDINDICES.y) ], _v ) * SEMANTIC_BLENDWEIGHTS.y; \
	_o += mul( BoneTransform_[ int(SEMANTIC_BLENDINDICES.z) ], _v ) * SEMANTIC_BLENDWEIGHTS.z; \
	_o += mul( BoneTransform_[ int(SEMANTIC_BLENDINDICES.w) ], _v ) * SEMANTIC_BLENDWEIGHTS.w;
#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v ) \
	_o =  vec4( mul( mat3( BoneTransform_[ int(SEMANTIC_BLENDINDICES.x) ] ), _v.xyz ) * SEMANTIC_BLENDWEIGHTS.x, 0.0 ); \
	_o += vec4( mul( mat3( BoneTransform_[ int(SEMANTIC_BLENDINDICES.y) ] ), _v.xyz ) * SEMANTIC_BLENDWEIGHTS.y, 0.0 ); \
	_o += vec4( mul( mat3( BoneTransform_[ int(SEMANTIC_BLENDINDICES.z) ] ), _v.xyz ) * SEMANTIC_BLENDWEIGHTS.z, 0.0 ); \
	_o += vec4( mul( mat3( BoneTransform_[ int(SEMANTIC_BLENDINDICES.w) ] ), _v.xyz ) * SEMANTIC_BLENDWEIGHTS.w, 0.0 ); \
	_o.xyz = normalize( _o.xyz );

#elif defined( PERM_MESH_PARTICLE_3D )

#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) \
	_o = _v + vec4( mul( mat3(InverseViewTransform_), SEMANTIC_TANGENT.xyz ), 1.0 );
#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v ) \
	_o = _v;

#elif defined( PERM_MESH_INSTANCED_3D )

// TODO: Properly.
#  define PSY_MAKE_WORLD_SPACE_VERTEX( _o, _v ) \
	_o = mul( WorldTransform_, _v );
#  define PSY_MAKE_WORLD_SPACE_NORMAL( _o, _v ) \
	_o = normalize( mul( NormalTransform_, _v ) );

#endif

////////////////////////////////////////////////////////////////////////
// PSY_MAKE_CLIP_SPACE_VERTEX
#if defined( PERM_MESH_STATIC_2D )

#  define PSY_MAKE_CLIP_SPACE_VERTEX( _o, _v ) \
		_o = _v;

#else

#  define PSY_MAKE_CLIP_SPACE_VERTEX( _o, _v ) \
		_o = mul( ClipTransform_, _v );

#endif

//////////////////////////////////////////////////////////////////////////
// Samplers.
#  define PSY_SAMPLER_1D( _n )									\
		uniform sampler1D a##_n									\

#  define PSY_SAMPLER_2D( _n )									\
		uniform sampler2D a##_n									\

#  define PSY_SAMPLER_3D( _n )									\
		uniform sampler3D a##_n									\

#  define PSY_SAMPLER_CUBE( _n )								\
		uniform samplerCube a##_n								\

#if ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_140 ) || ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_ES_300 )
#  define PSY_SAMPLE_1D( _n, _c ) texture( a##_n, _c )
#  define PSY_SAMPLE_2D( _n, _c ) texture( a##_n, _c )
#  define PSY_SAMPLE_3D( _n, _c ) texture( a##_n, _c )
#  define PSY_SAMPLE_CUBE( _n, _c ) texture( a##_n, _c )
#else
#  define PSY_SAMPLE_1D( _n, _c ) texture1D( a##_n, _c )
#  define PSY_SAMPLE_2D( _n, _c ) texture2D( a##_n, _c )
#  define PSY_SAMPLE_3D( _n, _c ) texture3D( a##_n, _c )
#  define PSY_SAMPLE_CUBE( _n, _c ) textureCUBE( a##_n, _c )
#endif

//////////////////////////////////////////////////////////////////////////
// Fragment writing.
#if PIXEL_SHADER

#if !defined( NOOF_OUTPUT_FRAGMENTS )
#  if defined( PERM_RENDER_DEFERRED )
#    define NOOF_MAX_OUTPUT_FRAGMENTS 4
#    define NOOF_OUTPUT_FRAGMENTS 4
#  else 
#    define NOOF_MAX_OUTPUT_FRAGMENTS 1
#    define NOOF_OUTPUT_FRAGMENTS 1
#  endif
#else
#  define NOOF_MAX_OUTPUT_FRAGMENTS NOOF_OUTPUT_FRAGMENTS
#endif

#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#  undef NOOF_MAX_OUTPUT_FRAGMENTS
#  define NOOF_MAX_OUTPUT_FRAGMENTS 4
#  define outputFrag gl_FragData
#  define FRAMEBUFFER_OUTPUT gl_FragData[0],  gl_FragData[1], gl_FragData[2], gl_FragData[3]
#  define FRAMEBUFFER_INPUT inout vec4 outFrag0, inout vec4 outFrag1, inout vec4 outFrag2, inout vec4 outFrag3
#  define FRAMEBUFFER_INTERNAL outFrag0, outFrag1, outFrag2, outFrag3

#elif PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_330
out float4 outputFrag[NOOF_MAX_OUTPUT_FRAGMENTS];
#  if NOOF_MAX_OUTPUT_FRAGMENTS == 1
#  define FRAMEBUFFER_OUTPUT outputFrag[0]
#  define FRAMEBUFFER_INPUT inout vec4 outFrag0
#  define FRAMEBUFFER_INTERNAL outFrag0
#  elif NOOF_MAX_OUTPUT_FRAGMENTS == 2
#  define FRAMEBUFFER_OUTPUT outputFrag[0], outputFrag[1]
#  define FRAMEBUFFER_INPUT inout vec4 outFrag0, inout vec4 outFrag1
#  define FRAMEBUFFER_INTERNAL outFrag0, outFrag1
#  elif NOOF_MAX_OUTPUT_FRAGMENTS == 3
#  define FRAMEBUFFER_OUTPUT outputFrag[0], outputFrag[1], outputFrag[2]
#  define FRAMEBUFFER_INPUT inout vec4 outFrag0, inout vec4 outFrag1, inout vec4 outFrag2
#  define FRAMEBUFFER_INTERNAL outFrag0, outFrag1, outFrag2
#  elif NOOF_MAX_OUTPUT_FRAGMENTS == 4
#  define FRAMEBUFFER_OUTPUT outputFrag[0], outputFrag[1], outputFrag[2], outputFrag[3]
#  define FRAMEBUFFER_INPUT inout vec4 outFrag0, inout vec4 outFrag1, inout vec4 outFrag2, inout vec4 outFrag3
#  define FRAMEBUFFER_INTERNAL outFrag0, outFrag1, outFrag2, outFrag3
#  endif
#endif

void clearFrag( FRAMEBUFFER_INPUT )
{
#if NOOF_OUTPUT_FRAGMENTS >= 1
	outFrag0 = vec4( 1.0, 1.0, 1.0, 1.0 );
#endif

#if NOOF_OUTPUT_FRAGMENTS >= 2
	outFrag1 = vec4( 1.0, 1.0, 1.0, 1.0 );
#endif

#if NOOF_OUTPUT_FRAGMENTS >= 3
	outFrag2 = vec4( 1.0, 1.0, 1.0, 1.0 );
#endif

#if NOOF_OUTPUT_FRAGMENTS >= 4
	outFrag3 = vec4( 0.0, 0.0, 0.0, 0.0 );
#endif
}

void writeFrag( FRAMEBUFFER_INPUT, in vec4 Colour )
{
	clearFrag( FRAMEBUFFER_INTERNAL );
	outFrag0 = Colour;
}

void writeFrag( FRAMEBUFFER_INPUT, in vec4 Albedo, in vec3 Normal )
{
	clearFrag( FRAMEBUFFER_INTERNAL );
	outFrag0.xyzw = Albedo.xyzw;
#if defined( PERM_RENDER_DEFERRED )
	outFrag2.xyz = ( Normal.xyz + vec3( 1.0, 1.0, 1.0 ) ) * 0.5;
#endif
}

void writeFrag( FRAMEBUFFER_INPUT, in vec4 Albedo, in vec3 Normal, in vec3 Material )
{
	clearFrag( FRAMEBUFFER_INTERNAL );
	outFrag0.xyzw = Albedo.xyzw;
#if defined( PERM_RENDER_DEFERRED )
	outFrag1.xyz = Material;
	outFrag2.xyz = ( Normal.xyz + vec3( 1.0, 1.0, 1.0 ) ) * 0.5;
#endif
}
void writeVelocity( FRAMEBUFFER_INPUT, in vec2 Velocity )
{
#if defined( PERM_RENDER_DEFERRED )
	outFrag3.xy = Velocity;
#endif
}

#endif // PIXEL_SHADER

//////////////////////////////////////////////////////////////////////////
// Uniforms.
#include <PsybrusUniforms.glsl>
#include <PsybrusLighting.glsl>

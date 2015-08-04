//////////////////////////////////////////////////////////////////////////
// Version directives.
#if PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_150
#version 150
#elif PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_330
#version 330
#elif PSY_OUTPUT_CODE_TYPE == PSY_CODE_TYPE_GLSL_ES_100
#version 100
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
// Input attributes.
#if ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_140 ) || ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_ES_300 )
#  define VS_IN( _Ty, _Name, _Semantic )		in _Ty _Name : _Semantic
#  define VS_OUT( _Ty, _Name )					out _Ty _Name

#  define PS_IN( _Ty, _Name )					in _Ty _Name

#else
#  define VS_IN( _Ty, _Name, _Semantic ) 		attribute _Ty _Name : _Semantic
#  define VS_OUT( _Ty, _Name )					varying _Ty _Name

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
		vec4( In[0].x, In[1].x, In[2].x, In[2].x ),
		vec4( In[0].y, In[1].y, In[2].y, In[2].y ),
		vec4( In[0].z, In[1].z, In[2].z, In[2].z ),
		vec4( In[0].w, In[1].w, In[2].w, In[2].w ) );
}
#endif

//////////////////////////////////////////////////////////////////////////
// HLSL mul.
vec3 mul( mat3 M, vec3 V )
{
	return V * transpose( M );
}

vec4 mul( mat4 M, vec4 V )
{
	return V * transpose( M );
}

//////////////////////////////////////////////////////////////////////////
// Samplers.
#  define PSY_SAMPLER_1D( _n )									\
		uniform sampler1D a##_n									\

#  define PSY_SAMPLER_2D( _n )									\
		uniform sampler2D a##_n									\

#  define PSY_SAMPLER_3D( _n )									\
		uniform sampler3D a##_n									\

#if ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_140 ) || ( PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES && PSY_OUTPUT_CODE_TYPE >= PSY_CODE_TYPE_GLSL_ES_300 )
#  define PSY_SAMPLE_1D( _n, _c ) texture( a##_n, _c )
#  define PSY_SAMPLE_2D( _n, _c ) texture( a##_n, _c )
#  define PSY_SAMPLE_3D( _n, _c ) texture( a##_n, _c )
#else
#  define PSY_SAMPLE_1D( _n, _c ) texture1D( a##_n, _c )
#  define PSY_SAMPLE_2D( _n, _c ) texture2D( a##_n, _c )
#  define PSY_SAMPLE_3D( _n, _c ) texture3D( a##_n, _c )
#endif
//////////////////////////////////////////////////////////////////////////
// Uniforms.
#include <PsybrusUniforms.glsl>

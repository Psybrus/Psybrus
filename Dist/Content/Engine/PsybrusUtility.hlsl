////////////////////////////////////////////////////////////////////////
// Matrix multiplication utilities. Help facilitate cross compilation.
float3 PsyMatMul( float3 Row0, float3 Row1, float3 Row2, float3 Vector )
{
	float3 RetVal = float3( 
		dot( Vector.xyz, Row0.xyz ),
		dot( Vector.xyz, Row1.xyz ),
		dot( Vector.xyz, Row2.xyz ) );
	return RetVal;
}

float4 PsyMatMul( float4 Row0, float4 Row1, float4 Row2, float4 Row3, float4 Vector )
{
	float4 RetVal = float4( 
		dot( Vector.xyzw, Row0 ),
		dot( Vector.xyzw, Row1 ),
		dot( Vector.xyzw, Row2 ),
		dot( Vector.xyzw, Row3 ) );
	return RetVal;
}

float3 PsyMatMul( float3x3 Mat, float3 Vector )
{
	return mul( Mat, Vector );
}

float4 PsyMatMul( float4x4 Mat, float4 Vector )
{
	return mul( Mat, Vector );
}

float3 PsyMatMulTranspose( float3 Row0, float3 Row1, float3 Row2, float3 Vector )
{
	float3 RetVal = Vector.xxx * Row0;
	RetVal += Vector.yyy * Row1;
	RetVal += Vector.zzz * Row2;
	return RetVal;
}

float4 PsyMatMulTranspose( float4 Row0, float4 Row1, float4 Row2, float4 Row3, float4 Vector )
{
	float4 RetVal = Vector.xxxx * Row0;
	RetVal += Vector.yyyy * Row1;
	RetVal += Vector.zzzz * Row2;
	RetVal += Vector.wwww * Row3;
	return RetVal;
}

////////////////////////////////////////////////////////////////////////
// Texture, samplers, and sampling.
#if 0 // PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES
#  define PSY_SAMPLER_1D( _n )									\
		sampler1D a##_n											\

#  define PSY_SAMPLER_2D( _n )									\
		sampler2D a##_n											\

#  define PSY_SAMPLER_3D( _n )									\
		sampler3D a##_n											\

#else

#  define PSY_SAMPLER_1D( _n )									\
		texture1D a##_n;										\
		SamplerState s##_n										\

#  define PSY_SAMPLER_2D( _n )									\
		texture2D a##_n;										\
		SamplerState s##_n										\

#  define PSY_SAMPLER_3D( _n )									\
		texture3D a##_n;										\
		SamplerState s##_n										\

#endif

#if 1 // PSY_OUTPUT_BACKEND_TYPE != PSY_BACKEND_TYPE_GLSL_ES
#  define PSY_SAMPLE_1D( _n, _c ) a##_n.Sample( s##_n, _c )
#  define PSY_SAMPLE_2D( _n, _c ) a##_n.Sample( s##_n, _c )
#  define PSY_SAMPLE_3D( _n, _c ) a##_n.Sample( s##_n, _c )

#else
#  define PSY_SAMPLE_1D( _n, _c ) tex1D( a##_n, _c )
#  define PSY_SAMPLE_2D( _n, _c ) tex2D( a##_n, _c )
#  define PSY_SAMPLE_3D( _n, _c ) tex3D( a##_n, _c )

#endif


////////////////////////////////////////////////////////////////////////
// Position + target defines for GLSL ES translation.
/*
#if PSY_OUTPUT_BACKEND_TYPE == PSY_BACKEND_TYPE_GLSL_ES
#  define SV_POSITION POSITION
#  define SV_POSITION0 POSITION0
#  define SV_POSITION1 POSITION1
#  define SV_POSITION2 POSITION2
#  define SV_POSITION3 POSITION3

#  define SV_TARGET COLOR0
#  define SV_TARGET0 COLOR0
#  define SV_TARGET1 COLOR1
#  define SV_TARGET2 COLOR2
#  define SV_TARGET3 COLOR3
#endif
*/

////////////////////////////////////////////////////////////////////////
// Unrolling utility.
#if ( PSY_INPUT_BACKEND_TYPE != PSY_BACKEND_TYPE_D3D11 ) && ( PSY_OUTPUT_BACKEND_TYPE != PSY_BACKEND_TYPE_GLSL_ES )
#  define PSY_UNROLL [unroll]
#else
#  define PSY_UNROLL
#endif

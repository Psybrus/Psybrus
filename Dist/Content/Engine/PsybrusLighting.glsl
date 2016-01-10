#pragma once

#include <PsybrusUniforms.glsl>

//////////////////////////////////////////////////////////////////////////
// DefaultPointLight
struct DefaultPointLight
{
	vec3 Position_;
	vec3 AttenuationCLQ_;
};

//////////////////////////////////////////////////////////////////////////
// calculateAttenuation
float calculateAttenuation( float Distance, vec3 Attenuation )
{
	float InvA = ( ( Attenuation.x + ( Distance * Attenuation.y ) + ( Distance * Distance * Attenuation.z ) ) );
	return 1.0 / InvA;
}

//////////////////////////////////////////////////////////////////////////
// Fresnel_SchlickApproximation
float Fresnel_SchlickApproximation( float F0, float CosA )
{
	float Pow5OneMinusCosA = pow( 1.0 - CosA, 5.0 );
	return F0 + ( 1.0 - F0 ) * Pow5OneMinusCosA;
}

//////////////////////////////////////////////////////////////////////////
// NDF_BlinnPhongNormalised
// Equation 19: https://dl.dropboxusercontent.com/u/55891920/papers/mm_brdf.pdf
float NDF_BlinnPhongNormalised( float NdotH, float N )
{
	float NormTerm = ( N + 2.0 ) / ( PIMUL2 );
	float SpecTerm = pow( NdotH, N );
	return NormTerm * SpecTerm;
}

//////////////////////////////////////////////////////////////////////////
// GeometryVisibility_CookTorrence
float GeometryVisibility_CookTorrence( float NdotL, float NdotV, float NdotH, float VdotH )
{
	float G = 1.0;
	G = min( G, ( 2.0 * NdotH * NdotV ) / VdotH );
	G = min( G, ( 2.0 * NdotH * NdotL ) / VdotH );
	return G;
}

//////////////////////////////////////////////////////////////////////////
// CookTorrence
float CookTorrence( float D, float F, float G, float NdotL, float NdotV )
{
	float Numerator = D * F * G;
	float Denominator = 4.0  * NdotL * NdotV;
	return Numerator / Denominator;
}

//////////////////////////////////////////////////////////////////////////
// Lambert
float Lambert( float NdotL )
{
	return max( NdotL, 0.0 );
}

//////////////////////////////////////////////////////////////////////////
// defaultLighting
void defaultLighting( int LightIdx, in vec3 EyePosition, in vec3 SurfacePosition, in vec3 Normal, inout vec3 OutDiffuse, inout vec3 OutSpecular )
{
	float EPSILON = 1e6;
	vec3 LightPosition = LightPosition_[ LightIdx ].xyz;
	vec3 ViewVector = normalize( EyePosition - SurfacePosition );
	vec3 LightVector = normalize( LightPosition - SurfacePosition );
	vec3 HalfVector = normalize( LightVector + ViewVector );
	float NdotL = max( 0.0, dot( Normal, LightVector ) );
	float NdotH = max( 0.0, dot( Normal, HalfVector ) );
	float NdotV = max( 0.0, dot( Normal, ViewVector ) );
	float LdotH = max( 0.0, dot( LightVector, HalfVector ) );
	float VdotH = max( 0.0, dot( ViewVector, HalfVector ) );

	float Reflectivity = 1.0;
	float Roughness = 0.0;
	float SpecularPower = ( ( 1.0 - Roughness ) * 100.0 ) + 1.0;

	// Specular.
#if 0
	float Specular = CookTorrence( 
		NDF_BlinnPhongNormalised( NdotH, SpecularPower ), 
		Fresnel_SchlickApproximation( Reflectivity, LdotH ),
		GeometryVisibility_CookTorrence( NdotL, NdotV, NdotH, VdotH ),
		NdotL, NdotV );
#else
	float Specular = min( pow( NdotH, SpecularPower ), 1.0 );
#endif

	// Diffuse
	float Diffuse = max( NdotL, 0.0 );
	Diffuse = 0.0;//Specular;

	OutDiffuse += vec3( Diffuse ) * LightDiffuseColour_[ LightIdx ].xyz + LightAmbientColour_[ LightIdx ].xyz;
	OutSpecular += vec3( Specular ) * LightDiffuseColour_[ LightIdx ].xyz;
}

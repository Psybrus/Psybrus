#pragma once

#include <PsybrusUniforms.glsl>

//////////////////////////////////////////////////////////////////////////
// Light
struct Light
{
	vec3 Position_;
	vec3 Colour_;
	vec3 AttenuationCLQ_;
};

//////////////////////////////////////////////////////////////////////////
// Material
struct Material
{
	vec3 Colour_;
	float Specular_;
	float Roughness_;
	float Metallic_;
};

//////////////////////////////////////////////////////////////////////////
// calculateAttenuation
float calculateAttenuation( float Distance, vec3 Attenuation )
{
	float InvA = ( ( Attenuation.x + ( Distance * Attenuation.y ) + ( Distance * Distance * Attenuation.z ) ) );
	return 1.0 / InvA;
}

//////////////////////////////////////////////////////////////////////////
// RoughnessToSpecularPower
float RoughnessToSpecularPower( float Roughness )
{
	return 1024.0 * pow( 1.0 - Roughness, 5.0 ) + 1.0;
}

//////////////////////////////////////////////////////////////////////////
// Fresnel_SchlickApproximation
vec3 Fresnel_SchlickApproximation( vec3 F0, float CosA )
{
	float Pow5OneMinusCosA = pow( 1.0 - CosA, 5.0 );
	return F0 + ( vec3( 1.0 ) - F0 ) * Pow5OneMinusCosA;
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
// NDF_GGX
// http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
float NDF_GGX( float NdotH, float Roughness )
{
	float Alpha = Roughness * Roughness;
	float AlphaSquared = Alpha * Alpha;
	float Denominator = NdotH * NdotH * ( Alpha - 1.0 ) + 1.0;
	return AlphaSquared / ( PI * Denominator * Denominator );
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
vec3 CookTorrence( float D, vec3 F, float G, float NdotL, float NdotV )
{
	vec3 Numerator = vec3( D * F * G );
	vec3 Denominator = vec3( 4.0  * NdotL * NdotV );
	return Numerator / Denominator;
}

//////////////////////////////////////////////////////////////////////////
// BRDF_Default
vec3 BRDF_Default( Light InLight, in Material InMaterial, in vec3 ViewPosition, in vec3 SurfacePosition, in vec3 Normal, in vec3 ReflectionColour )
{
	vec3 LightPosition = InLight.Position_.xyz;
	vec3 ViewVector = normalize( ViewPosition - SurfacePosition );
	vec3 LightVector = normalize( LightPosition - SurfacePosition );
	vec3 HalfVector = normalize( LightVector + ViewVector );
	float SmallValue = 0.00001;
	float NdotL = max( SmallValue, dot( Normal, LightVector ) );
	float NdotH = max( SmallValue, dot( Normal, HalfVector ) );
	float NdotV = max( SmallValue, dot( Normal, ViewVector ) );
	float LdotH = max( SmallValue, dot( LightVector, HalfVector ) );
	float VdotH = max( SmallValue, dot( ViewVector, HalfVector ) );

#if 0	
	// SurfacePosition - good.
	// ViewPosition - good.
	// Normal - good
	// ViewVector - good
	// NdotL - 
	// NdotH - 
	vec3 RetVal = mod( vec3( ( Normal + 1.0 ) * 0.5 ), vec3( 1.0 ) );
#if defined ( PERM_RENDER_DEFERRED )
	return RetVal;
#else
	return RetVal;
#endif
#endif

	// Convert roughness to specular power.
	float Roughness = InMaterial.Roughness_;

	// Calculate reflectance.
	vec3 SpecularReflectance = mix( vec3( InMaterial.Specular_ ), InMaterial.Colour_, InMaterial.Metallic_ ); 

	// Calculate terms for spec + diffuse.
#if 1
	float D = NDF_BlinnPhongNormalised( NdotH, RoughnessToSpecularPower( Roughness ) );
#else
	float D = NDF_GGX( NdotH, Roughness );
#endif
	float G = GeometryVisibility_CookTorrence( NdotL, NdotV, NdotH, VdotH );
	vec3 Fspec = Fresnel_SchlickApproximation( SpecularReflectance, LdotH );

	// Specular.
	vec3 Specular = CookTorrence( D, Fspec, G, NdotL, NdotV );

	// Diffuse
	vec3 Diffuse = vec3( max( NdotL, 0.0 ) );
	Diffuse = max( vec3( 0.0 ), Diffuse * ( vec3( 1.0 ) - Fspec ) * ( vec3( 1.0 - InMaterial.Metallic_ ) ) );

	// Specular colour.
	vec3 SpecularColour = mix( Specular * InMaterial.Colour_, ReflectionColour, InMaterial.Metallic_ );

	// Total colour.
	vec3 Total = ( Diffuse * ( InMaterial.Colour_ / vec3( PI ) ) ) + SpecularColour;

	// Punctual light source.
	return Total * InLight.Colour_ * NdotL;
}

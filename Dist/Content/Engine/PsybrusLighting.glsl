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
// defaultLighting
void defaultLighting( in vec3 Normal, out vec3 OutDiffuse, out vec3 OutSpecular )
{
	// Hacky lighting.
	vec3 CameraVector = mul( ViewTransform_, vec4( 0.0, 0.0, 1.0, 0.0 ) ).xyz;
	vec3 LightVector = -normalize( vec3( cos( ViewTime_.x ), -1.0, sin( ViewTime_.x ) ) );
	vec3 HalfVector = normalize( LightVector + CameraVector );
	float NdotL = dot( Normal.xyz, LightVector );
	float NdotH = dot( Normal.xyz, HalfVector );
	
	// Ambient + Diffuse
	float DiffuseLight = max( NdotL, 0.0 );
	DiffuseLight = min( DiffuseLight, 1.0 );

	float Facing = NdotL > 0.0 ? 1.0 : 0.0;
	float SpecularLight = ( Facing * pow( max( NdotH, 0.0 ), 1.0 ) ) * 0.2;	

	OutDiffuse = vec3( DiffuseLight );
	OutSpecular = vec3( SpecularLight );
}

varying vec4 vTexCoord0;
varying vec4 vEyePosition;
varying vec4 vWorldPosition;

uniform sampler3D aDiffuseTex;

const float AttenuationMultiplier = 0.25;
const float InterpolationMultiplier = 0.06125;

void main()
{
	float Distance = length( vWorldPosition - vEyePosition );
	float Attenuation = clamp( ( Distance - 0.25 ) * AttenuationMultiplier, 0.0, 2.0 );
	float Interpolate = clamp( ( Distance - 0.25 ) * InterpolationMultiplier, 0.0, 1.0 );
	float Colour = texture3D( aDiffuseTex, vTexCoord0.xyz ).w;
	vec3 NearColour = vec3( 0.25, 0.25, 1.0 );
	vec3 FarColour = vec3( 1.0, 0.25, 0.25 );
	vec3 RenderColour = mix( NearColour, FarColour, Interpolate ) * Colour * Attenuation;
	gl_FragColor = vec4( RenderColour.x, RenderColour.y, RenderColour.z, 1.0 );
}

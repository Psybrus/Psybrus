varying vec4 vTexCoord0;

uniform sampler3D aDiffuseTex;

vec4 shaderMain()
{
	vec4 Colour = texture3D( aDiffuseTex, vTexCoord0.xyz ) * 1.0;
	return vec4( Colour.w * 0.5, Colour.w * 0.5, Colour.w, 1.0 ) + vec4(0.02,0.02,0.02,0.0);
}

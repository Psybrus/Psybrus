// particle.glslf

varying vec4 vTexCoord0;

uniform sampler2D aDiffuseTex;

vec4 shaderMain()			
{					
	vec3 TexColour = texture2D( aDiffuseTex, vTexCoord0.xy ).xyz;	
	return vec4( ( TexColour * vColour.xyz ), TexColour.r * vColour.w );
}

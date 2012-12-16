// particle.glslf

varying vec4 vTexCoord0;

uniform sampler2D aDiffuseTex;

vec4 shaderMain()			
{						
	return vec4( ( texture2D( aDiffuseTex, vTexCoord0.xy ).xyz * vColour.xyz ), 1.0 );
}

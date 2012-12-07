varying vec4 vTexCoord0;

uniform sampler2D aDiffuseTex;	

uniform vec2 aAlphaTestStep;

vec4 shaderMain()
{						
	vec4 Colour = texture2D( aDiffuseTex, vTexCoord0.xy );
	vec2 AlphaTest = aAlphaTestStep;
	float Factor = smoothstep( AlphaTest.x, AlphaTest.y, Colour.a );
	return vec4( Colour.rgb, Factor ) * vColour;
}

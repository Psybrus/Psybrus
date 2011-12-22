varying vec4 vColour;
varying vec4 vTexCoord0;
uniform sampler2D aDiffuseTex;	
uniform vec2 aAlphaTestStep;

void main()			
{						
	vec4 Colour = texture2D( aDiffuseTex, vTexCoord0.xy );

	vec2 AlphaTest = aAlphaTestStep;
	float Factor = smoothstep( AlphaTest.x, AlphaTest.y, Colour.a );

	//gl_FragColor = vec4( 1.0, 1.0, 1.0, Colour.a ) * vColour;
	gl_FragColor = vec4( Colour.rgb, Factor ) * vColour;
}

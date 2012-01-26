varying vec4 vColour;
varying vec4 vTexCoord0;
varying vec4 vPosition;

uniform sampler2D aNoiseTex;
uniform float uInverseDrawDistance;

void main()			
{					
	float FogFactor = min( 1.0 - ( vPosition.z * uInverseDrawDistance ), 0.0 );
	vec4 Colour = vec4( FogFactor, FogFactor, FogFactor, 1.0 ) * vColour;
	Colour = Colour + vec4( 0.51, 0.51, 0.51, 0.0 );
	Colour = Colour * texture2D( aNoiseTex, ( vTexCoord0.xy  / ( vTexCoord0.z * 0.25 ) ) * vec2( 2.0, 2.0 ) );
	Colour = Colour * 0.25;
	gl_FragColor = Colour;
}

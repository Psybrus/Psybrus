varying vec4 vColour;
varying vec4 vTexCoord0;

uniform sampler2D aDiffuseTex;

void main()			
{						
	gl_FragColor = vec4( ( texture2D( aDiffuseTex, vTexCoord0.xy ).xyz * vColour.xyz ) * vColour.w, 1.0 );
}

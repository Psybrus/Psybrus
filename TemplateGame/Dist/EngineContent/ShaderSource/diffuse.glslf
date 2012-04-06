varying vec4 vColour;
varying vec4 vNormal;
varying vec4 vTexCoord0;

uniform sampler2D aDiffuseTex;

void main()			
{						
	gl_FragColor = vec4( vec3( 0.5, 0.5, 0.5 ) + vNormal.xyz * 0.5, 1.0 );
}

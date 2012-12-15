varying vec4 vColour;
varying vec4 vNormal;
varying vec4 vTexCoord0;

uniform sampler2D aDiffuseTex;

void main()
{						
	gl_FragColor = vColour;
}

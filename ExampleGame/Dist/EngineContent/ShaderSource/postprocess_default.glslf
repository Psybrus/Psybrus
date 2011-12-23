varying vec4 vColour;
varying vec4 vTexCoord0;

// Textures.
uniform sampler2D aDiffuseTex;

void main()			
{						
	gl_FragColor = texture2D( aDiffuseTex, vTexCoord0.xy ) * vColour;
}

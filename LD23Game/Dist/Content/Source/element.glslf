varying vec4 vColour;
varying vec4 vNormal;
varying vec4 vTexCoord0;
varying vec4 vVisibleMapTexCoord;

uniform sampler2D aDiffuseTex;
uniform sampler2D aVisibleMapTex;

void main()			
{					
	vec4 DiffuseColour = texture2D( aDiffuseTex, vTexCoord0.xy ) * vColour;
	vec4 VisibleMapColour = texture2D( aVisibleMapTex, vVisibleMapTexCoord.xy );	
	gl_FragColor = vec4( DiffuseColour.xyz, DiffuseColour.w * VisibleMapColour.x );
}

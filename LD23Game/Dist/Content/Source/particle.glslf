varying vec4 vColour;
varying vec4 vTexCoord0;
varying vec4 vHeatMapTexCoord;

uniform sampler2D aDiffuseTex;
uniform sampler2D aHeatMapTex;
uniform sampler2D aVisibleMapTex;

void main()			
{					
	vec4 DiffuseColour = texture2D( aDiffuseTex, vTexCoord0.xy );
	vec4 HeatMapColour = texture2D( aHeatMapTex, vHeatMapTexCoord.xy );
	vec4 VisibleMapColour = texture2D( aVisibleMapTex, vHeatMapTexCoord.xy );
	gl_FragColor = vec4( DiffuseColour.xyz * vColour.xyz * HeatMapColour, 1.0) * VisibleMapColour;
}

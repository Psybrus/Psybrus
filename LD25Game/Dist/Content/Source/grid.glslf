uniform sampler2D aDiffuseTex;

varying vec4 vUVScrolling;
varying vec4 vTexCoord0;

vec4 shaderMain()
{
	vec4 Colour = texture2D( aDiffuseTex, vTexCoord0.xy + vUVScrolling.xy ).xyzw * vColour.xyzw;

	return Colour;
}

uniform sampler2D aDiffuseTex;	

vec4 shaderMain()
{
	return texture2D( aDiffuseTex, vTexCoord0.xy ) * vColour;
}

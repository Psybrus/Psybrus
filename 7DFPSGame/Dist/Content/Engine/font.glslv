varying vec4 vTexCoord0;

vec4 shaderMain(vec4 inPosition)
{
	vTexCoord0 = aTexCoord0;
	return inPosition;
}

varying vec4 vTexCoord0;

vec4 shaderMain(vec4 inPosition)
{
	vec4 Vertex = aPosition;
	gl_Position = aPosition;
	vTexCoord0 = aTexCoord0;
	vTexCoord0.z = 0.5;

	return inPosition;
}

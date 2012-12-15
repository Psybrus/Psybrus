attribute vec4 aTexCoord0;

uniform vec4 uUVScrolling;

varying vec4 vUVScrolling;
varying vec4 vTexCoord0;

vec4 shaderMain(vec4 inPosition)
{
	vTexCoord0 = aTexCoord0;
	vUVScrolling = uUVScrolling;
	return inPosition;
}

attribute vec4 aTexCoord0;

varying vec3 vEyePosition;
varying vec4 vWorldPosition;
varying vec4 vTexCoord0;

vec4 shaderMain(vec4 inPosition)
{
	vEyePosition = uEyePosition;
	vWorldPosition = uWorldTransform * aPosition;
	vTexCoord0 = aTexCoord0;

	return inPosition;
}

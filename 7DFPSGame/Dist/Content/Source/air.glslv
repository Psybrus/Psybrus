#version 120

attribute vec4 aPosition;
attribute vec4 aTexCoord0;
uniform mat4 uWorldTransform;
uniform mat4 uClipTransform;
uniform vec3 uEyePosition;

varying vec3 vEyePosition;
varying vec4 vWorldPosition;
varying vec4 vTexCoord0;
void main()
{
	vEyePosition = uEyePosition;
	vWorldPosition = uWorldTransform * aPosition;
	vTexCoord0 = aTexCoord0;

	gl_Position = ( uClipTransform * vWorldPosition );
}

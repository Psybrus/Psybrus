#version 120

attribute vec4 aPosition;
attribute vec4 aNormal;
attribute vec4 aTangent;
attribute vec4 aTexCoord0;
attribute vec4 aTexCoord1;
attribute vec4 aTexCoord2;
attribute vec4 aTexCoord3;
attribute vec4 aColour;

uniform mat4 uClipTransform;
uniform mat4 uViewTransform;
uniform mat4 uInverseViewTransform;
uniform mat4 uWorldTransform;
uniform vec3 uEyePosition;

varying vec4 vColour;

vec4 shaderMain();

void main()
{
	gl_FragColor = shaderMain();
}

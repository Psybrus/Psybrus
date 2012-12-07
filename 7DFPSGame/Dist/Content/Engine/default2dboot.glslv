#version 120

attribute vec4 aPosition;
attribute vec4 aColour;

uniform mat4 uWorldTransform;
uniform mat4 uClipTransform;

varying vec4 vColour;

vec4 shaderMain(vec4 inPosition);

void main()
{
	vColour = aColour;
	gl_Position = shaderMain(aPosition);
}

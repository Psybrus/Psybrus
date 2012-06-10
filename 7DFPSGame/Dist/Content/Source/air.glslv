#version 120

attribute vec4 aPosition;
attribute vec4 aTexCoord0;
uniform mat4 uWorldTransform;
uniform mat4 uClipTransform;
varying vec4 vTexCoord0;

void main()
{
	vec4 Vertex = aPosition;
	gl_Position = ( uClipTransform * ( uWorldTransform * Vertex ) );
	vTexCoord0 = aTexCoord0;
}
#version 120

attribute vec4 aPosition;
attribute vec4 aNormal;
attribute vec4 aTexCoord0;
attribute vec4 aColour;

uniform mat4 uWorldTransform;
uniform mat4 uClipTransform;

varying vec4 vNormal;
varying vec4 vTexCoord0;
varying vec4 vColour;

void main()
{
	vec4 Vertex = aPosition ;
	gl_Position = ( uClipTransform * ( uWorldTransform * Vertex ) );
	vNormal = aNormal;
	vTexCoord0 = aTexCoord0;
	vColour = aColour;
}

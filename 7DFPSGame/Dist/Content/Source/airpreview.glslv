#version 120

attribute vec4 aPosition;
attribute vec4 aTexCoord0;
varying vec4 vTexCoord0;

void main()
{
	vec4 Vertex = aPosition;
	gl_Position = aPosition;
	vTexCoord0 = aTexCoord0;
	vTexCoord0.z = 0.5;
}
#version 120

attribute vec4 aPosition;
attribute vec4 aNormal;
attribute vec4 aTexCoord0;
attribute vec4 aColour;

uniform mat4 uInverseViewTransform;
uniform mat4 uViewTransform;
uniform mat4 uWorldTransform;
uniform mat4 uClipTransform;

varying vec4 vTexCoord0;
varying vec4 vColour;

void main()
{
	mat3 BillboardTransform = mat3( uInverseViewTransform[0].xyz, uInverseViewTransform[1].xyz, uInverseViewTransform[2].xyz );
	vec4 Offset = vec4( BillboardTransform * aNormal.xyz, 0.0 );
	vec4 Vertex = aPosition + Offset;
	gl_Position = ( uClipTransform * ( uWorldTransform * Vertex ) );
	vTexCoord0 = aTexCoord0;
	vColour = aColour;
}

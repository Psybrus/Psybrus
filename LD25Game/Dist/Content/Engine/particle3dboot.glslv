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

vec4 shaderMain(vec4 inPosition);

void main()
{
	mat3 BillboardTransform = mat3( uInverseViewTransform[0].xyz, uInverseViewTransform[1].xyz, uInverseViewTransform[2].xyz );
	vec4 Offset = vec4( BillboardTransform * aNormal.xyz, 0.0 );
	vec4 Vertex = aPosition + Offset;
	vColour = aColour;

	gl_Position = shaderMain( uClipTransform * ( uWorldTransform * Vertex ) );
}

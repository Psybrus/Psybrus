#version 120

varying vec4 vColour;

uniform mat4 uClipTransform;
uniform mat4 uViewTransform;
uniform mat4 uInverseViewTransform;
uniform mat4 uWorldTransform;
uniform vec3 uEyePosition;

vec4 shaderMain();

void main()
{						
	gl_FragColor = shaderMain();
}

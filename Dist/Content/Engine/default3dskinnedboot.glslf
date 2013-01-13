#version 150

uniform mat4 uClipTransform;
uniform mat4 uViewTransform;
uniform mat4 uInverseViewTransform;
uniform mat4 uWorldTransform;
uniform vec3 uEyePosition;
uniform mat4 uBoneTransform[24];

in vec4 vColour;
in vec4 vTexCoord0;
in vec4 vTexCoord1;
in vec4 vTexCoord2;
in vec4 vTexCoord3;

vec4 shaderMain();

void main()
{						
	gl_FragColor = shaderMain();
}

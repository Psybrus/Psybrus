#version 120

varying vec4 vColour;

vec4 shaderMain();

void main()
{						
	gl_FragColor = shaderMain();
}

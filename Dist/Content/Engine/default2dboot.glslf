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

in vec4 aPosition;
in vec4 aNormal;
in vec4 aTangent;
in vec4 aTexCoord0;
in vec4 aTexCoord1;
in vec4 aTexCoord2;
in vec4 aTexCoord3;
in vec4 aColour;

out vec4 vColour;
out vec4 vTexCoord0;
out vec4 vTexCoord1;
out vec4 vTexCoord2;
out vec4 vTexCoord3;

vec4 shaderMain(vec4 inPosition);

void main()
{
	vec4 Vertex = aPosition;
	vec4 TransformedVertex = ( uClipTransform * ( uWorldTransform * Vertex ) );
	vColour = aColour;
 	vTexCoord0 = aTexCoord0;
 	vTexCoord1 = aTexCoord1;
 	vTexCoord2 = aTexCoord2;
 	vTexCoord3 = aTexCoord3;
 	
	gl_Position = shaderMain( TransformedVertex );
}

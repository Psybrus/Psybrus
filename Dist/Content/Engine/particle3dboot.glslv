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
	mat3 BillboardTransform = mat3( uInverseViewTransform[0].xyz, uInverseViewTransform[1].xyz, uInverseViewTransform[2].xyz );
	vec4 Offset = vec4( BillboardTransform * aNormal.xyz, 0.0 );
	vec4 Vertex = aPosition + Offset;
	vColour = aColour;
 	vTexCoord0 = aTexCoord0;
 	vTexCoord1 = aTexCoord1;
 	vTexCoord2 = aTexCoord2;
 	vTexCoord3 = aTexCoord3;

	gl_Position = shaderMain( uClipTransform * ( uWorldTransform * Vertex ) );
}

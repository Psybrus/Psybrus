in vec4 aPosition;
in vec4 aNormal;
in vec4 aTangent;
in vec4 aTexCoord0;
in vec4 aTexCoord1;
in vec4 aTexCoord2;
in vec4 aTexCoord3;
in vec4 aSkinIndices;
in vec4 aSkinWeights;
in vec4 aColour;

out vec4 vColour;
out vec4 vTexCoord0;
out vec4 vTexCoord1;
out vec4 vTexCoord2;
out vec4 vTexCoord3;

vec4 shaderMain(vec4 inPosition);

void main()
{
	vec4 Vertex = vec4( 0.0, 0.0, 0.0, 0.0 );
	Vertex += ( uBoneTransform[ int(aSkinIndices.x) ] * aPosition ) * aSkinWeights.x;
	Vertex += ( uBoneTransform[ int(aSkinIndices.y) ] * aPosition ) * aSkinWeights.y;
	Vertex += ( uBoneTransform[ int(aSkinIndices.z) ] * aPosition ) * aSkinWeights.z;
	Vertex += ( uBoneTransform[ int(aSkinIndices.w) ] * aPosition ) * aSkinWeights.w;

 	vTexCoord0 = aTexCoord0;
 	vTexCoord1 = aTexCoord1;
 	vTexCoord2 = aTexCoord2;
 	vTexCoord3 = aTexCoord3;

	vec4 TransformedVertex = ( uClipTransform * Vertex );
	vColour = aColour;

	gl_Position = shaderMain( TransformedVertex );
}

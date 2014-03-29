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
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBinormal;
out vec4 vTexCoord0;
out vec4 vTexCoord1;
out vec4 vTexCoord2;
out vec4 vTexCoord3;
out vec3 vLightDirection[4];
out float vLightDistance[4];

vec4 shaderMain(vec4 inPosition);

void main()
{
	vec4 Position = vec4( 0.0, 0.0, 0.0, 0.0 );
	Position += ( uBoneTransform[ int(aSkinIndices.x) ] * aPosition ) * aSkinWeights.x;
	Position += ( uBoneTransform[ int(aSkinIndices.y) ] * aPosition ) * aSkinWeights.y;
	Position += ( uBoneTransform[ int(aSkinIndices.z) ] * aPosition ) * aSkinWeights.z;
	Position += ( uBoneTransform[ int(aSkinIndices.w) ] * aPosition ) * aSkinWeights.w;

	vec3 Normal = vec3( 0.0, 0.0, 0.0 );
	Normal += ( mat3( uBoneTransform[ int(aSkinIndices.x) ] ) * aNormal.xyz ) * aSkinWeights.x; // TODO Inv transpose?
	Normal += ( mat3( uBoneTransform[ int(aSkinIndices.y) ] ) * aNormal.xyz ) * aSkinWeights.y; // TODO Inv transpose?
	Normal += ( mat3( uBoneTransform[ int(aSkinIndices.z) ] ) * aNormal.xyz ) * aSkinWeights.z; // TODO Inv transpose?
	Normal += ( mat3( uBoneTransform[ int(aSkinIndices.w) ] ) * aNormal.xyz ) * aSkinWeights.w; // TODO Inv transpose?
	vNormal = normalize( Normal );

	vec3 Tangent = vec3( 0.0, 0.0, 0.0 );
	Tangent += ( mat3( uBoneTransform[ int(aSkinIndices.x) ] ) * aTangent.xyz ) * aSkinWeights.x; // TODO Inv transpose?
	Tangent += ( mat3( uBoneTransform[ int(aSkinIndices.y) ] ) * aTangent.xyz ) * aSkinWeights.y; // TODO Inv transpose? 
	Tangent += ( mat3( uBoneTransform[ int(aSkinIndices.z) ] ) * aTangent.xyz ) * aSkinWeights.z; // TODO Inv transpose?
	Tangent += ( mat3( uBoneTransform[ int(aSkinIndices.w) ] ) * aTangent.xyz ) * aSkinWeights.w; // TODO Inv transpose?
	vTangent = Tangent;

	vBinormal = cross( Normal, Tangent );

	// Calculate light direction and distance.
	for(int i = 0; i < 4; ++i)
	{
		vLightDirection[i] = normalize( Position.xyz - uLightPosition[i].xyz );
		vLightDistance[i] = distance( Position.xyz, uLightPosition[i].xyz );
	}

 	vColour = aColour;
 	vTexCoord0 = aTexCoord0;
 	vTexCoord1 = aTexCoord1;
 	vTexCoord2 = aTexCoord2;
 	vTexCoord3 = aTexCoord3;

	vec4 TransformedVertex = ( uClipTransform * Position );
	gl_Position = shaderMain( TransformedVertex );
}

in vec4 aPosition;
in vec4 aNormal;
in vec4 aTangent;
in vec4 aTexCoord0;
in vec4 aTexCoord1;
in vec4 aTexCoord2;
in vec4 aTexCoord3;
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
	vec4 Position = uWorldTransform * aPosition;
	vNormal = mat3( uWorldTransform ) * aNormal.xyz;		// TODO Inv transpose.
	vColour = aColour;
 	vTexCoord0 = aTexCoord0;
 	vTexCoord1 = aTexCoord1;
 	vTexCoord2 = aTexCoord2;
 	vTexCoord3 = aTexCoord3;

 	// Calculate light direction and distance.
	for(int i = 0; i < 4; ++i)
	{
		vLightDirection[i] = normalize( Position.xyz - uLightPosition[i].xyz );
		vLightDistance[i] = distance( Position.xyz, uLightPosition[i].xyz );
	}
 	
	vec4 TransformedVertex = ( uClipTransform * Position );
	gl_Position = shaderMain( TransformedVertex );
}

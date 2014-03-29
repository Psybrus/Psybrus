in vec4 vColour;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBinormal;
in vec4 vTexCoord0;
in vec4 vTexCoord1;
in vec4 vTexCoord2;
in vec4 vTexCoord3;
in vec3 vLightDirection[4];
in float vLightDistance[4];

vec4 shaderMain();

void main()
{
	vec4 UnlitColour = shaderMain();

	// Calculate lighting.
	vec3 Normal = normalize( vNormal );
	vec4 FinalDiffuse = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i = 0; i < 4; ++i)
	{
		float Distance = vLightDistance[ i ];
		float Attn = 1.0 / ( uLightAttn[i].x + ( Distance * uLightAttn[i].y ) + ( Distance * Distance * uLightAttn[i].z ) );
		float NdotL = max( dot( Normal, normalize( vLightDirection[i] ) ), 0 );
		FinalDiffuse += uLightAmbientColour[i] + clamp( uLightDiffuseColour[i] * NdotL * Attn, 0.0, 1.0 );
	}

	gl_FragColor = UnlitColour * clamp( FinalDiffuse, 0.0, 1.0 );
}

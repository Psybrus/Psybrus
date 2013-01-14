#version 150

uniform mat4 uClipTransform;
uniform mat4 uViewTransform;
uniform mat4 uInverseViewTransform;
uniform mat4 uWorldTransform;
uniform vec3 uEyePosition;
uniform mat4 uBoneTransform[24];

uniform vec3 uLightPosition[4];
uniform vec3 uLightDirection[4];
uniform vec4 uLightAmbientColour[4];
uniform vec4 uLightDiffuseColour[4];
uniform vec3 uLightAttn[4];

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

uniform sampler2D aNormalTex;

vec4 shaderMain();

void main()
{
	vec4 UnlitColour = shaderMain();

	// Generate TBN matrix.
	mat3 TBNMatrix = mat3( normalize( vTangent ), normalize( vBinormal ), normalize( vNormal ) );

	// Calculate lighting.
	// NORMAL MAPPING: vec3 Normal = ( TBNMatrix * normalize( texture2D( aNormalTex, vTexCoord0.xy ).xyz ) );
	vec3 Normal = ( TBNMatrix * vec3( 0.0f, 0.0f, 1.0f ) );
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

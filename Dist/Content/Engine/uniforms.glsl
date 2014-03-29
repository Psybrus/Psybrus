#version 150

#define USE_UBOS
#ifdef USE_UBOS
	#define BEGIN_UBO( __name ) layout(std140) uniform __name {
	#define END_UBO };
#else
	#define BEGIN_UBO( __name )
	#define END_UBO
#endif

//BEGIN_UBO( ViewUniformBlock )
	uniform mat4 uClipTransform;
	uniform mat4 uViewTransform;
	uniform mat4 uInverseViewTransform;
	uniform mat4 uWorldTransform;
	uniform vec3 uEyePosition;
//END_UBO

//BEGIN_UBO( LightUniformBlock )
	uniform vec3 uLightPosition[4];
	uniform vec3 uLightDirection[4];
	uniform vec4 uLightAmbientColour[4];
	uniform vec4 uLightDiffuseColour[4];
	uniform vec3 uLightAttn[4];
//END_UBO

BEGIN_UBO( BoneUniformBlock )
	uniform mat4 uBoneTransform[24];
END_UBO

//BEGIN_UBO( AlphaTestBlock )
	uniform vec2 aAlphaTestStep;
//END_UBO

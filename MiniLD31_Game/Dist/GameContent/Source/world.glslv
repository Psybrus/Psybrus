attribute vec4 aPosition;
attribute vec4 aColour;
attribute vec4 aTexCoord0;

uniform mat4 uWorldTransform;
uniform mat4 uClipTransform;
uniform mat4 uViewTransform;

varying vec4 vColour;
varying vec4 vTexCoord0;
varying vec4 vPosition;

void main()
{
	vec4 Vertex = aPosition;
	vec4 ClipSpaceVertex = ( uClipTransform * ( uWorldTransform * Vertex ) );
	gl_Position = ClipSpaceVertex;
	vPosition = ( uViewTransform * ( uWorldTransform * Vertex ) );
	vColour = aColour;
	vTexCoord0 = ClipSpaceVertex;
}

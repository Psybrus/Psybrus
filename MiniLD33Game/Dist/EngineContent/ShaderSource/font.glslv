attribute vec4 aPosition;
attribute vec4 aColour;
attribute vec4 aTexCoord0;
varying vec4 vColour;
varying vec4 vTexCoord0;

void main()
{
	gl_Position = aPosition;

	vColour = aColour;

	vTexCoord0 = aTexCoord0;
}

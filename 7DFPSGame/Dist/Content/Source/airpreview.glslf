varying vec4 vTexCoord0;

uniform sampler3D aDiffuseTex;	

void main()
{
	vec4 Colour = texture3D( aDiffuseTex, vTexCoord0.xyz ) * 1.0;
	gl_FragColor = vec4( Colour.w * 0.5, Colour.w * 0.5, Colour.w, 1.0 ) + vec4(0.02,0.02,0.02,0.0);
}

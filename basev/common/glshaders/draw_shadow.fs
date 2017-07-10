#version 110

uniform sampler2D Texture;
uniform float Alpha;

varying vec2 TextureCoordinate;

void main ()
{
	float Transp;

	Transp = clamp (((
		(texture2D (Texture, TextureCoordinate).w * Alpha)
		- 0.1) / 0.9), 0.0, 1.0);

	vec4 FinalColour;
	FinalColour.xyz = vec3(0.0, 0.0, 0.0);
	FinalColour.w = (Transp * (Transp * (3.0 - 
		(2.0 * Transp)
		)));

	gl_FragColor = FinalColour;
}

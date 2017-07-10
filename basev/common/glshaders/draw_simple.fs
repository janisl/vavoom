#version 110

uniform sampler2D Texture;
uniform float Alpha;

varying vec2 TextureCoordinate;

void main ()
{
	vec4 FinalColour;
	vec4 TexCol;

	TexCol = texture2D (Texture, TextureCoordinate);
	FinalColour.xyz = TexCol.xyz;

	if ((TexCol.w < 0.4))
	{
		discard;
	};
	float Transp;

	Transp = clamp (((Alpha - 0.4) / 0.6), 0.0, 1.0);

	FinalColour.w = (TexCol.w * (Transp * (Transp * 
		(3.0 - (2.0 * Transp))
		)));

	gl_FragColor = FinalColour;
}

#version 110

uniform sampler2D Texture;

varying vec2 TextureCoordinate;

void main ()
{
	vec4 FinalColour_1;
	vec4 TexColour;

	TexColour = texture2D (Texture, TextureCoordinate);
	FinalColour_1.xyz = TexColour.xyz;

	if ((TexColour.w < 0.1))
	{
		discard;
	};
	float ClampTransp;

	ClampTransp = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
	FinalColour_1.w = (TexColour.w * (ClampTransp * (ClampTransp * 
		(3.0 - (2.0 * ClampTransp))
		)));

	gl_FragColor = FinalColour_1;
}

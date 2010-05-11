uniform sampler2D	Texture;

varying vec2		TextureCoordinate;

void main()
{
	vec4 FinalColour = texture2D(Texture, TextureCoordinate);
	if (FinalColour.a <= 0.666)
	{
		discard;
	}

	gl_FragColor = FinalColour;
}

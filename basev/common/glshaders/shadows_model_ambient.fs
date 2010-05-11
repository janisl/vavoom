uniform sampler2D	Texture;
uniform vec4		Light;

varying vec2		TextureCoordinate;

void main()
{
	vec4 TexColour = texture2D(Texture, TextureCoordinate);
	if (TexColour.a <= 0.666)
	{
		discard;
	}
	gl_FragColor = Light;
}

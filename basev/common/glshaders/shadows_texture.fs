#version 110

uniform sampler2D	Texture;

varying vec2		TextureCoordinate;

void main()
{
	vec4 TexColour = texture2D(Texture, TextureCoordinate);
	if (TexColour.a < 0.1)
	{
		discard;
	}
	TexColour.a *= smoothstep(0.1, 1.0, TexColour.a);
	gl_FragColor = TexColour;
}

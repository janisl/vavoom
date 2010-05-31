#version 110

uniform sampler2D	Texture;
uniform float		Alpha;

varying vec2		TextureCoordinate;

void main()
{
	vec4 FinalColour = texture2D(Texture, TextureCoordinate);
	if (FinalColour.a < 0.666)
	{
		discard;
	}
	FinalColour.a *= Alpha;
	gl_FragColor = FinalColour;
}

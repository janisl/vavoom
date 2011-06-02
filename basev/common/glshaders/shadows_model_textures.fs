#version 110

uniform sampler2D	Texture;

varying vec2		TextureCoordinate;

uniform float		InAlpha;

void main()
{
	vec4 FinalColour = texture2D(Texture, TextureCoordinate);
	if (FinalColour.a <= 0.333)
	{
		discard;
	}

	gl_FragColor = vec4(FinalColour.rgb, InAlpha * FinalColour.a);
}

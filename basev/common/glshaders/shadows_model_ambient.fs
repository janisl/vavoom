#version 110

uniform sampler2D	Texture;
uniform vec4		Light;

varying vec2		TextureCoordinate;

uniform float		InAlpha;

void main()
{
	vec4 TexColour = texture2D(Texture, TextureCoordinate);
	if (TexColour.a <= 0.666)
	{
		discard;
	}

	gl_FragColor = vec4(Light.rgb, InAlpha * TexColour.a);
}

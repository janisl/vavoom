#version 110

uniform sampler2D	Texture;
uniform float		Alpha;

varying vec2		TextureCoordinate;

void main()
{
	vec4 TexColour = texture2D(Texture, TextureCoordinate);
	gl_FragColor = vec4(0.0, 0.0, 0.0, smoothstep(0.1, 1.0, TexColour.a * Alpha));
}

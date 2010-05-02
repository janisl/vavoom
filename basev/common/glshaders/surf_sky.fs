uniform sampler2D	Texture;
uniform float		Brightness;

varying vec2		TextureCoordinate;

void main()
{
	gl_FragColor = texture2D(Texture, TextureCoordinate) * vec4(Brightness, Brightness, Brightness, 1.0);
}

uniform sampler2D	Texture;

varying vec2		TextureCoordinate;

void main()
{
	gl_FragColor = texture2D(Texture, TextureCoordinate);
}

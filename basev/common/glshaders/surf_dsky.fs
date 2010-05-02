uniform sampler2D	Texture;
uniform sampler2D	Texture2;
uniform float		Brightness;

varying vec2		TextureCoordinate;
varying vec2		Texture2Coordinate;

void main()
{
	vec4 Tex1 = texture2D(Texture, TextureCoordinate);
	vec4 Tex2 = texture2D(Texture2, Texture2Coordinate);
	gl_FragColor = mix(Tex1, Tex2, Tex2.a) * vec4(Brightness, Brightness, Brightness, 1.0);
}

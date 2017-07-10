#version 110

uniform sampler2D Texture;
uniform float Brightness;

varying vec2 TextureCoordinate;

void main ()
{
	vec4 BrightFactor;

	BrightFactor.w = 1.0;
	BrightFactor.x = Brightness;
	BrightFactor.y = Brightness;
	BrightFactor.z = Brightness;

	gl_FragColor = (texture2D (Texture, TextureCoordinate) * BrightFactor);
}

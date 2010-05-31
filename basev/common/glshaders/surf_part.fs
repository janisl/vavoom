#version 110

varying vec4		Light;
varying vec2		TextureCoordinate;

void main()
{
	float a = clamp((1.0 - length(TextureCoordinate)) * 2.0, 0.0, 1.0);
	if (a <= 0.0)
	{
		discard;
	}
	gl_FragColor = Light * (1.0, 1.0, 1.0, a);
}

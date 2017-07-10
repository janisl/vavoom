#version 110

varying vec4 Light;
varying vec2 TextureCoordinate;

void main ()
{
	float a;

	a = clamp (((1.0 - 
		sqrt(dot (TextureCoordinate, TextureCoordinate))
		) * 2.0), 0.0, 1.0);

	if ((a < 0.1))
	{
		discard;
	};
	float Transp;

	Transp = clamp (((a - 0.1) / 0.9), 0.0, 1.0);
	gl_FragColor = (Light * (Transp * (Transp * 
		(3.0 - (2.0 * Transp))
		)));
}

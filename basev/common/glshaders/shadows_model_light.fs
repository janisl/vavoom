#version 110

uniform sampler2D	Texture;
uniform vec3		LightColour;
uniform float		LightRadius;

varying vec3		Normal;
//varying float		Dist;
varying vec3		VertToLight;
varying vec2		TextureCoordinate;

void main()
{
	vec4 TexColour = texture2D(Texture, TextureCoordinate);
	if (TexColour.a <= 0.666)
	{
		discard;
	}

	float DistToLight = length(VertToLight); //* 100.0;
	if (DistToLight >= LightRadius)
	{
		discard;
	}

	vec3 Incoming = normalize(VertToLight);
	vec3 Norm = normalize(Normal);
	float Angle = dot(Incoming, Norm);
//	Angle = 0.5 + 0.5 * Angle;

//	float Add = LightRadius - Dist;
	float Add = LightRadius - DistToLight;
	Add *= Angle;
	Add = clamp(Add / 255.0, 0.0, 1.0);
	if (Add <= 0.0)
	{
		discard;
	}

	gl_FragColor = vec4(LightColour.r, LightColour.g, LightColour.b, Add * TexColour.a);
//	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

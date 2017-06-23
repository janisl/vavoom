#version 110

uniform sampler2D	Texture;
uniform vec3		LightColour;
uniform float		LightRadius;

varying vec3		Normal;
varying float		Dist;
varying vec3		VertToLight;
varying vec3		VertToView;
varying vec2		TextureCoordinate;

void main()
{
	float DistToView = length(VertToView);
	if (DistToView <= 0.0)
	{
		discard;
	}
	if (Dist <= 0.0)
	{
		discard;
	}

	float DistToLight = length(VertToLight); //* 100.0;
	if (DistToLight <= 0.0)
	{
		discard;
	}
	if (DistToLight > LightRadius)
	{
		discard;
	}

	vec4 TexColour = texture2D(Texture, TextureCoordinate);
	if (TexColour.a < 0.1)
	{
		discard;
	}

	vec3 Incoming = normalize(VertToLight);
	float Angle = dot(Incoming, Normal);
	Angle = 0.5 + 0.5 * Angle;
	//float Add = LightRadius - Dist;
	float Add = LightRadius - DistToLight;
	Add *= Angle;
	if (Add <= 0.0)
	{
		discard;
	}
	Add = clamp(Add / 255.0, 0.0, 1.0);

	gl_FragColor = vec4(LightColour.rgb, Add * smoothstep(0.1, 1.0, TexColour.a));
//	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

uniform vec3		LightColour;
uniform float		LightRadius;

varying vec3		Normal;
varying float		Dist;
varying vec3		VertToLight;

void main()
{
	float DistToLight = length(VertToLight);
	if (DistToLight >= LightRadius)
	{
		discard;
	}
	vec3 Incoming = normalize(VertToLight);
	float Angle = dot(Incoming, Normal);
	Angle = 0.5 + 0.5 * Angle;
	//float Add = LightRadius - Dist;
	float Add = LightRadius - DistToLight;
	Add *= Angle;
	//if (Add <= 0.0)
	//{
	//	discard;
	//}
	Add = clamp(Add / 255.0, 0.0, 1.0);

	gl_FragColor = vec4(LightColour.r, LightColour.g, LightColour.b, Add);
}

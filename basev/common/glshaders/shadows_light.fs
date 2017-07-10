#version 110

uniform vec3 LightColour;
uniform float LightRadius;
uniform sampler2D Texture;

varying vec3 Normal;
varying vec3 VertToLight;
varying vec3 VertToView;
varying float Dist;
varying float VDist;
varying vec2 TextureCoordinate;

void main ()
{
	float Add_1;
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));

	if ((DistToView <= 0.0))
	{
		discard;
	};
	if ((VDist <= 0.0))
	{
		discard;
	};

	if ((Dist <= 0.0))
	{
		discard;
	};
	float DistToLight;

	DistToLight = sqrt(dot (VertToLight, VertToLight));
	if ((DistToLight <= 0.0))
	{
		discard;
	};

	if ((DistToLight > LightRadius))
	{
		discard;
	};
	vec4 TexColour;

	TexColour = texture2D (Texture, TextureCoordinate);

	if ((TexColour.w < 0.1))
	{
		discard;
	};
	Add_1 = ((LightRadius - DistToLight) * (0.5 + (0.5 * 
			dot (normalize(VertToLight), Normal)
			)));

	if ((Add_1 <= 0.0))
	{
		discard;
	};
	float ClampAdd;

	ClampAdd = clamp ((Add_1 / 255.0), 0.0, 1.0);
	Add_1 = ClampAdd;
	float Transp;

	Transp = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
	vec4 FinalColour;

	FinalColour.xyz = LightColour;
	FinalColour.w = (ClampAdd * (Transp * (Transp * 
					(3.0 - (2.0 * Transp))
					)));

	gl_FragColor = FinalColour;
}


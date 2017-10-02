#version 110
uniform sampler2D Texture;
uniform vec3 LightColour;
uniform float LightRadius;
uniform float InAlpha;
uniform bool AllowTransparency;

varying vec3 Normal;
varying vec3 VertToLight;
varying vec3 VertToView;
varying vec3 VPos;
varying vec3 VPosL;
varying vec2 TextureCoordinate;
varying float Dist;
varying float VDist;

void main ()
{
	vec4 FinalColour;
	float Add;
	float DistVPosL;

	DistVPosL = sqrt(dot (VPosL, VPosL));
	float DistVPos;

	DistVPos = sqrt(dot (VPos, VPos));

	if ((Dist > 0.0))
	{
		if ((DistVPosL < -(LightRadius)))
		{
			discard;
		};

		if ((DistVPos < 0.0))
		{
			discard;
		};
	}
	else
	{
		if ((DistVPosL > LightRadius))
		{
			discard;
		};

		if ((DistVPos > 0.0))
		{
			discard;
		};
	};
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));

	if ((Dist > 0.0))
	{
		if ((VDist < 0.0))
		{
			discard;
		};
		if ((DistToView < 0.0))
		{
			discard;
		};
	}
	else
	{
		if ((VDist > 0.0))
		{
			discard;
		};
		if ((DistToView > 0.0))
		{
			discard;
		};
	};
	float DistToLight;

	DistToLight = sqrt(dot (VertToLight, VertToLight));

	if ((Dist > 0.0))
	{
		if ((DistToLight > LightRadius))
		{
			discard;
		};
	}
	else
	{
		if ((DistToLight < -(LightRadius)))
		{
			discard;
		};
	};
	vec4 TexColour;

	TexColour = texture2D (Texture, TextureCoordinate);

	if ((TexColour.w < 0.1))
	{
		discard;
	};
	Add = ((LightRadius - DistToLight) * (0.5 + (0.5 * 
	dot (normalize(VertToLight), Normal)
	)));

	if ((Add <= 0.0))
	{
		discard;
	};
	float ClampAdd;

	ClampAdd = clamp ((Add / 255.0), 0.0, 1.0);
	Add = ClampAdd;
	float ClampTrans;

	ClampTrans = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
	FinalColour.xyz = LightColour;
	FinalColour.w = ((ClampAdd * TexColour.w) * (ClampTrans * (ClampTrans * 
	(3.0 - (2.0 * ClampTrans))
	)));

	if ((AllowTransparency == bool(0)))
	{
		if (((InAlpha == 1.0) && (ClampTrans < 0.666)))
		{
			discard;
		};
	}
	else
	{
		if ((ClampTrans < 0.1))
		{
			discard;
		};
	};
	gl_FragColor = FinalColour;
}
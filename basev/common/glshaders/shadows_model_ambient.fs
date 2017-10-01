#version 110

uniform vec4 Light;
uniform sampler2D Texture;
uniform float InAlpha;
uniform bool AllowTransparency;

varying vec2 TextureCoordinate;
varying vec3 VertToView;
varying vec3 VPos;
varying float Dist;

void main ()
{
	vec4 FinalColour;
	float DistVPos;

	DistVPos = sqrt(dot (VPos, VPos));

	if (((Dist > 0.0) && (DistVPos < 0.0)))
	{
		discard;
	};
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));
	vec4 TexColour;

	TexColour = texture2D (Texture, TextureCoordinate);

	if ((TexColour.w < 0.1))
	{
		discard;
	};

	if ((Dist >= 0.0))
	{
		if ((DistToView < 0.0))
		{
			vec4 DarkColour;
			float ClampTransp;

			ClampTransp = clamp (((
			(Light.w * TexColour.w)
			- 0.1) / 0.9), 0.0, 1.0);
			DarkColour.xyz = (Light.xyz * 0.75);
			DarkColour.w = (InAlpha * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

			if ((AllowTransparency == bool(0)))
			{
				if (((InAlpha == 1.0) && (ClampTransp < 0.666)))
				{
					discard;
				};
			}
			else
			{
				if ((ClampTransp < 0.1))
				{
					discard;
				};
			};
			FinalColour = DarkColour;
		}
		else
		{
			vec4 BrightColour;
			float ClampTransp;

			ClampTransp = clamp (((
			(Light.w * TexColour.w)
			- 0.1) / 0.9), 0.0, 1.0);
			BrightColour.xyz = Light.xyz;
			BrightColour.w = (InAlpha * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

			if ((AllowTransparency == bool(0)))
			{
				if (((InAlpha == 1.0) && (ClampTransp < 0.666)))
				{
					discard;
				};
			}
			else
			{
				if ((ClampTransp < 0.1))
				{
					discard;
				};
			};
			FinalColour = BrightColour;
		};
	}
	else
	{
		if ((DistToView > 0.0))
		{
			vec4 DarkColour;
			float ClampTransp;

			ClampTransp = clamp (((
			(Light.w * TexColour.w)
			- 0.1) / 0.9), 0.0, 1.0);
			DarkColour.xyz = (Light.xyz * 0.75);
			DarkColour.w = (InAlpha * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

			if ((AllowTransparency == bool(0)))
			{
				if (((InAlpha == 1.0) && (ClampTransp < 0.666)))
				{
					discard;
				};
			}
			else
			{
				if ((ClampTransp < 0.1))
				{
					discard;
				};
			};
			FinalColour = DarkColour;
		}
		else
		{
			vec4 BrightColour;
			float ClampTransp;

			ClampTransp = clamp (((
			(Light.w * TexColour.w)
			- 0.1) / 0.9), 0.0, 1.0);
			BrightColour.xyz = Light.xyz;
			BrightColour.w = (InAlpha * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

			if ((AllowTransparency == bool(0)))
			{
				if (((InAlpha == 1.0) && (ClampTransp < 0.666)))
				{
					discard;
				};
			}
			else
			{
				if ((ClampTransp < 0.1))
				{
					discard;
				};
			};
			FinalColour = BrightColour;
		};
	};
	gl_FragColor = FinalColour;
}
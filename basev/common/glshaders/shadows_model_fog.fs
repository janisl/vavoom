#version 110

uniform vec4 FogColour;
uniform sampler2D Texture;
uniform float FogDensity;
uniform float FogStart;
uniform float FogEnd;
uniform int FogType;

varying vec3 VertToView;
varying vec2 TextureCoordinate;
varying float Dist;

uniform float InAlpha;

void main ()
{
	vec4 MixColour_1;
	float FogFactor_2;
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));

	if ((DistToView <= 0.0))
	{
		discard;
	};
	vec4 FinalColour;

	FinalColour = texture2D (Texture, TextureCoordinate);
	
	if ((FinalColour.w <= 0.1))
	{
		discard;
	};
	float z;

	z = (gl_FragCoord.z / gl_FragCoord.w);

	if ((FogType == 3))
	{
		FogFactor_2 = exp2(((
			((-(FogDensity) * FogDensity) * z)
			* z) * 1.442695));
	}
	else
	{
		if ((FogType == 2))
		{
			FogFactor_2 = exp2(((
				-(FogDensity)
				* z) * 1.442695));
		}
		else
		{
			FogFactor_2 = ((FogEnd - z) / (FogEnd - FogStart));
		};
	};

	if ((Dist <= 0.0))
	{
		FogFactor_2 = (clamp ((1.0 - FogFactor_2), 0.0, 1.0) * InAlpha);
		float ClampTransp;

		ClampTransp = clamp (((FinalColour.w - 0.1) / 0.9), 0.0, 1.0);
		vec4 DarkColour;

		DarkColour.xyz = FogColour.xyz;
		DarkColour.w = (FogFactor_2 * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

		MixColour_1 = DarkColour;
	}
	else
	{
		FogFactor_2 = (clamp ((0.85 - FogFactor_2), 0.0, 0.85) * InAlpha);
		float ClampTransp;

		ClampTransp = clamp (((FinalColour.w - 0.1) / 0.9), 0.0, 1.0);
		vec4 BrightColour;

		BrightColour.xyz = (FogColour.xyz * 0.85);
		BrightColour.w = (FogFactor_2 * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

		MixColour_1 = BrightColour;
	};

	gl_FragColor = MixColour_1;
}

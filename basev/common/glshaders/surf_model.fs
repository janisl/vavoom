#version 110

uniform sampler2D Texture;
uniform vec4 FogColour;
uniform float FogDensity;
uniform float FogStart;
uniform float FogEnd;
uniform float InAlpha;
uniform int FogType;
uniform bool FogEnabled;
uniform bool AllowTransparency;

varying vec4 Light;
varying vec3 VertToView;
varying vec3 VPos;
varying vec2 TextureCoordinate;

void main ()
{
	vec4 FinalColour;
	vec4 TexColour;
	float DistVPos;
	
	DistVPos = sqrt(dot (VPos, VPos));
	
	if ((DistVPos < 0.0))
	{
		discard;
	};
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));

	if ((DistToView < 0.0))
	{
		discard;
	};
	TexColour = (texture2D (Texture, TextureCoordinate) * Light);
	FinalColour.xyz = TexColour.xyz;
	FinalColour.w = TexColour.w;

	if ((TexColour.w < 0.1))
	{
		discard;
	};

	if (FogEnabled)
	{
		float z;
		float FogFactor;

		z = (gl_FragCoord.z / gl_FragCoord.w);

		if ((FogType == 3))
		{
			FogFactor = exp2(((
			((-(FogDensity) * FogDensity) * z)
			* z) * 1.442695));
		}
		else
		{
			if ((FogType == 2))
			{
				FogFactor = exp2(((
				  -(FogDensity)
				 * z) * 1.442695));
			}
			else
			{
				FogFactor = ((FogEnd - z) / (FogEnd - FogStart));
			};
		};
		float ClampFogFactor;

		ClampFogFactor = clamp (FogFactor, 0.0, 1.0);
		FogFactor = ClampFogFactor;

		float ClampTransp;

		ClampTransp = clamp (((ClampFogFactor - 0.1) / 0.9), 0.0, 1.0);
		FinalColour.xyz = mix (FogColour.xyz, TexColour.xyz, (ClampTransp * (ClampTransp * 
		  (3.0 - (2.0 * ClampTransp))
		)));
		FinalColour.w = mix (FogColour.w, TexColour.w, (ClampTransp * (ClampTransp * 
		  (3.0 - (2.0 * ClampTransp))
		)));
	};

	if ((AllowTransparency == bool(0)))
	{
		if (((InAlpha == 1.0) && (FinalColour.w < 0.666)))
		{
			discard;
		};
	}
	else
	{
		if ((FinalColour.w < 0.1))
		{
			discard;
		};
	};
	gl_FragColor = FinalColour;
}
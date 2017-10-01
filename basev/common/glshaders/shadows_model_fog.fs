#version 110

uniform vec4 FogColour;
uniform sampler2D Texture;
uniform float FogDensity;
uniform float FogStart;
uniform float FogEnd;
uniform int FogType;
uniform bool AllowTransparency;

varying vec3 VertToView;
varying vec3 VPos;
varying vec2 TextureCoordinate;
varying float Dist;
uniform float InAlpha;

void main ()
{
	float z;
	float FogFactor;
	vec4 FinalColour;
	float DistVPos;

	DistVPos = sqrt(dot (VPos, VPos));

	if ((Dist > 0.0))
	{
		if ((DistVPos < 0.0))
		{
			discard;
		};
	}
	else
	{
		if ((DistVPos > 0.0))
		{
			discard;
		};
	};
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));

	vec4 TexColour;

	TexColour = texture2D (Texture, TextureCoordinate);

	if ((TexColour.w < 0.1))
	{
		discard;
	};
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

	if ((Dist >= 0.0))
	{
		if ((DistToView <= 0.0))
		{
			vec4 DarkColour;

			FogFactor = (clamp ((1.0 - FogFactor), 0.0, 1.0) * InAlpha);
			float ClampTrans;

			ClampTrans = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
			DarkColour.xyz = FogColour.xyz;
			DarkColour.w = ((FogFactor * InAlpha) * (ClampTrans * (ClampTrans * 
			(3.0 - (2.0 * ClampTrans))
			)));
			FinalColour = DarkColour;
		}
		else
		{
			vec4 BrightColour;

			FogFactor = (clamp ((0.75 - FogFactor), 0.0, 0.75) * InAlpha);
			float ClampTrans;

			ClampTrans = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
			BrightColour.xyz = (FogColour.xyz * 0.75);
			BrightColour.w = ((FogFactor * InAlpha) * (ClampTrans * (ClampTrans * 
			(3.0 - (2.0 * ClampTrans))
			)));
			FinalColour = BrightColour;
		};
	}
	else
	{
		if ((DistToView >= 0.0))
		{
			vec4 DarkColour;

			FogFactor = (clamp ((1.0 - FogFactor), 0.0, 1.0) * InAlpha);
			float ClampTrans;

			ClampTrans = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
			DarkColour.xyz = FogColour.xyz;
			DarkColour.w = ((FogFactor * InAlpha) * (ClampTrans * (ClampTrans * 
			(3.0 - (2.0 * ClampTrans))
			)));
			FinalColour = DarkColour;
		}
		else
		{
			vec4 BrighColour;

			FogFactor = (clamp ((0.75 - FogFactor), 0.0, 0.75) * InAlpha);
			float ClampTrans;

			ClampTrans = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
			BrighColour.xyz = (FogColour.xyz * 0.75);
			BrighColour.w = ((FogFactor * InAlpha) * (ClampTrans * (ClampTrans * 
			(3.0 - (2.0 * ClampTrans))
			)));
			FinalColour = BrighColour;
		};
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
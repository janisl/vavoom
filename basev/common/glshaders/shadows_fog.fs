#version 110

uniform int FogType;
uniform vec4 FogColour;
uniform float FogDensity;
uniform float FogStart;
uniform float FogEnd;

void main ()
{
	float FogFactor;
	float z;

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

	ClampFogFactor = clamp ((1.0 - FogFactor), 0.0, 1.0);
	FogFactor = ClampFogFactor;

	float SmoothFactor;

	SmoothFactor = clamp (((ClampFogFactor - 0.1) / 0.9), 0.0, 1.0);
	vec4 FinalFogColour;

	FinalFogColour.xyz = FogColour.xyz;
	FinalFogColour.w = (SmoothFactor * (SmoothFactor * (3.0 - 
					(2.0 * SmoothFactor)
					)));

	gl_FragColor = FinalFogColour;
}

#version 110

uniform sampler2D Texture;
uniform sampler2D LightMap;
uniform sampler2D SpecularMap;
uniform bool FogEnabled;
uniform int FogType;
uniform vec4 FogColour;
uniform float FogDensity;
uniform float FogStart;
uniform float FogEnd;

varying vec2 TextureCoordinate;
varying vec2 LightmapCoordinate;

void main ()
{
	vec4 FinalColour_1;
	vec4 TexColour;

	TexColour = ((texture2D (Texture, TextureCoordinate) * texture2D (LightMap, LightmapCoordinate)) + texture2D (SpecularMap, LightmapCoordinate));
	FinalColour_1 = TexColour;

	if ((TexColour.w < 0.1))
	{
		discard;
	};

	if (FogEnabled)
	{
		float FogFactor_3;
		float z;

		z = (gl_FragCoord.z / gl_FragCoord.w);

		if ((FogType == 3))
		{
			FogFactor_3 = exp2(((
				((-(FogDensity) * FogDensity) * z)
				* z) * 1.442695));
		}
		else
		{
			if ((FogType == 2))
			{
				FogFactor_3 = exp2(((
					-(FogDensity)
					* z) * 1.442695));
			}
			else
			{
				FogFactor_3 = ((FogEnd - z) / (FogEnd - FogStart));
			};
		};
		float SmoothFactor;

		SmoothFactor = clamp (FogFactor_3, 0.0, 1.0);
		FogFactor_3 = SmoothFactor;
		float FogFactor;

		FogFactor = clamp (((SmoothFactor - 0.1) / 0.9), 0.0, 1.0);
		FinalColour_1 = mix (FogColour, TexColour, (FogFactor * (FogFactor * 
			(3.0 - (2.0 * FogFactor))
			)));
	};

	gl_FragColor = FinalColour_1;
}

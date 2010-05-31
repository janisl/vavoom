#version 110

uniform sampler2D	Texture;
uniform sampler2D	LightMap;
uniform sampler2D	SpecularMap;
uniform bool		FogEnabled;
uniform int			FogType;
uniform vec4		FogColour;
uniform float		FogDensity;
uniform float		FogStart;
uniform float		FogEnd;

varying vec2		TextureCoordinate;
varying vec2		LightmapCoordinate;

void main()
{
	vec4 FinalColour = texture2D(Texture, TextureCoordinate) * texture2D(LightMap, LightmapCoordinate) +
		 texture2D(SpecularMap, LightmapCoordinate);

	if (FogEnabled)
	{
		float z = gl_FragCoord.z / gl_FragCoord.w;
		const float LOG2 = 1.442695;
		float FogFactor;
		if (FogType == 3)
		{
			FogFactor = exp2(-FogDensity * FogDensity * z * z * LOG2);
		}
		else if (FogType == 2)
		{
			FogFactor = exp2(-FogDensity * z * LOG2);
		}
		else
		{
			FogFactor = (FogEnd - z) / (FogEnd - FogStart);
		}
		FogFactor = clamp(FogFactor, 0.0, 1.0);
		FinalColour = mix(FogColour, FinalColour, FogFactor);
	}

	gl_FragColor = FinalColour;
}

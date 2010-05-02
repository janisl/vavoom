uniform sampler2D	Texture;
uniform bool		FogEnabled;
uniform int			FogType;
uniform vec4		FogColour;
uniform float		FogDensity;
uniform float		FogStart;
uniform float		FogEnd;

varying vec4		Light;
varying vec2		TextureCoordinate;

void main()
{
	vec4 FinalColour = texture2D(Texture, TextureCoordinate) * Light;
	if (FinalColour.a <= 0.0)
	{
		discard;
	}

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

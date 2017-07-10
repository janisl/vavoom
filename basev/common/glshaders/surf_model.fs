#version 110

uniform sampler2D Texture;
uniform vec4 FogColour;
uniform float FogDensity;
uniform float FogStart;
uniform float FogEnd;
uniform int FogType;
uniform bool FogEnabled;

varying vec4 Light;
varying vec3 VertToView;
varying vec2 TextureCoordinate;

void main ()
{
	vec4 FinalColour_1;
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));

	if ((DistToView <= 0.0))
	{
		discard;
	};
	vec4 TexColour;

	TexColour = (texture2D (Texture, TextureCoordinate) * Light);
	FinalColour_1 = TexColour;

	if ((TexColour.w < 0.1))
	{
		discard;
	};
	
	if (FogEnabled)
	{
		float FogFactor_4;
		float z;

		z = (gl_FragCoord.z / gl_FragCoord.w);

		if ((FogType == 3))
		{
			FogFactor_4 = exp2(((
				((-(FogDensity) * FogDensity) * z)
				* z) * 1.442695));
		}
		else
		{
			if ((FogType == 2))
			{
				FogFactor_4 = exp2(((
					-(FogDensity)
					* z) * 1.442695));
			}
			else
			{
				FogFactor_4 = ((FogEnd - z) / (FogEnd - FogStart));
			};
		};
		float ClampFactor;

		ClampFactor = clamp (FogFactor_4, 0.0, 1.0);
		FogFactor_4 = ClampFactor;

		float FogFactor;
		FogFactor = clamp (((ClampFactor - 0.1) / 0.9), 0.0, 1.0);

		FinalColour_1 = mix (FogColour, TexColour, (FogFactor * (FogFactor * 
			(3.0 - (2.0 * FogFactor))
			)));
	};

	gl_FragColor = FinalColour_1;
}

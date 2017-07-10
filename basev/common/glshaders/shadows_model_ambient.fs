#version 110

uniform vec4 Light;
uniform sampler2D Texture;
uniform float InAlpha;

varying vec2 TextureCoordinate;
varying vec3 VertToView;
varying float Dist;

void main ()
{
	vec4 FinalColor_1;
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));

	if ((DistToView <= 0.0))
	{
		discard;
	};
	vec4 TexColour;

	TexColour = texture2D (Texture, TextureCoordinate);

	if ((TexColour.w < 0.1))
	{
		discard;
	};

	if ((Dist <= 0.0))
	{
		float ClampTransp;

		ClampTransp = clamp (((
			(Light.w * TexColour.w)
			- 0.1) / 0.9), 0.0, 1.0);
		vec4 DarkColour;

		DarkColour.xyz = (Light.xyz * 0.85);
		DarkColour.w = (InAlpha * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

		FinalColor_1 = DarkColour;
	}
	else
	{
		float ClampTransp;

		ClampTransp = clamp (((
			(Light.w * TexColour.w)
			- 0.1) / 0.9), 0.0, 1.0);
		vec4 BrightColour;

		BrightColour.xyz = Light.xyz;
		BrightColour.w = (InAlpha * (ClampTransp * (ClampTransp * 
			(3.0 - (2.0 * ClampTransp))
			)));

		FinalColor_1 = BrightColour;
  };

  gl_FragColor = FinalColor_1;
}

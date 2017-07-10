#version 110

uniform sampler2D Texture;
uniform float InAlpha;

varying vec3 VertToView;
varying vec2 TextureCoordinate;

void main ()
{
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
	float ClampTransp;

	ClampTransp = clamp (((TexColour.w - 0.1) / 0.9), 0.0, 1.0);
	vec4 FinalColour;

	FinalColour.xyz = TexColour.xyz;
	FinalColour.w = (InAlpha * (ClampTransp * (ClampTransp * 
		(3.0 - (2.0 * ClampTransp))
		)));

	gl_FragColor = FinalColour;
}

#version 110

uniform sampler2D	Texture;
uniform float		InAlpha;

varying vec2		TextureCoordinate;
varying vec3		VertToView;

void main()
{
	float DistToView = length(VertToView);

	if (DistToView <= 0.0)
	{
		discard;
	}

	vec4 FinalColour = texture2D(Texture, TextureCoordinate);

	if (FinalColour.a < 0.1)
	{
		discard;
	}

	gl_FragColor = vec4(FinalColour.rgb, InAlpha * smoothstep(0.1, 1.0, FinalColour.a));
}

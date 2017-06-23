#version 110

uniform sampler2D	Texture;
uniform vec4		Light;

varying vec2		TextureCoordinate;
varying vec3		VertToView;

uniform float		InAlpha;

void main()
{
	/*float DistToView = length(VertToView);

	if (DistToView <= 0.0)
	{
		discard;
	}*/

	vec4 TexColour = texture2D(Texture, TextureCoordinate);
	if (TexColour.a < 0.1)
	{
		discard;
	}

	gl_FragColor = vec4(Light.rgb, InAlpha * smoothstep(0.1, 1.0, Light.a * TexColour.a));
}

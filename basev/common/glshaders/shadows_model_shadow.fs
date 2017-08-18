#version 110

uniform float LightRadius;

varying vec3 VertToView;
//varying vec3 VertToLight;
//varying float Offs;

void main ()
{
	float DistToView;

	DistToView = sqrt(dot (VertToView, VertToView));
	if ((DistToView <= 0.0))
	{
		discard;
	};

	/*if ((Offs > 0.0))
	{
		float DistToLight;

		DistToLight = sqrt(dot (VertToLight, VertToLight));
		if ((DistToLight > LightRadius))
		{
			discard;
		};
	};*/

	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}

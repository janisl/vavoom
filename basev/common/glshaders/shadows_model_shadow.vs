#version 110

uniform mat4 ModelToWorldMat;
uniform vec3 LightPos;
uniform vec3 ViewOrigin;
uniform float Inter;

attribute vec4 Vert2;
attribute float Offset;

varying vec3 VertToView;
//varying vec3 VertToLight;
//varying float Offs;

void main ()
{
	vec4 Vert_1;

	Vert_1 = (mix (gl_Vertex, Vert2, Inter) * ModelToWorldMat);
	if ((Offset > 0.0))
	{
		Vert_1.xyz = (LightPos + (Offset * normalize(
			(Vert_1.xyz - LightPos)
			)));
	};
	gl_Position = (gl_ModelViewProjectionMatrix * Vert_1);

	//VertToLight = (LightPos - Vert_1.xyz);
	VertToView = (ViewOrigin - Vert_1.xyz);
	//Offs = Offset;
}


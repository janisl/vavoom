#version 110

uniform float		Inter;
uniform vec3		LightPos;
uniform mat4		ModelToWorldMat;

attribute vec4		Vert2;
attribute float		Offset;

void main()
{
	vec4 Vert = mix(gl_Vertex, Vert2, Inter);
	Vert = Vert * ModelToWorldMat;
	if (Offset > 0.0)
	{
		vec3 Dir = normalize(Vert.xyz - LightPos);
//		Vert += Offset * vec4(Dir, 0.0);
		Vert.xyz = LightPos + Offset * Dir;
	}
	gl_Position = gl_ModelViewProjectionMatrix * Vert;
}

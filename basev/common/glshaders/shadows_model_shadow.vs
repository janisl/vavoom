#version 110

uniform mat4 ModelToWorldMat;
uniform vec3 LightPos;
uniform vec3 ViewOrigin;
uniform float Inter;

attribute vec4 Vert2;
attribute float Offset;

varying vec3 VertToView;
varying vec3 VPosL;
varying vec3 VPos;
varying float Offs;

void main ()
{
	vec4 Vert;
	Vert = (mix (gl_Vertex, Vert2, Inter) * ModelToWorldMat);

	if ((Offset > 0.0))
	{
		Vert.xyz = (LightPos + (Offset * normalize(
		  (Vert.xyz - LightPos)
		)));
	};
	gl_Position = (gl_ModelViewProjectionMatrix * Vert);
	VertToView = (ViewOrigin - Vert.xyz);
	VPosL = (LightPos - gl_Position.xyz);
	VPos = (ViewOrigin - gl_Position.xyz);
	Offs = Offset;
}
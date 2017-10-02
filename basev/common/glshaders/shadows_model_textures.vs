#version 110

uniform mat4 ModelToWorldMat;
uniform mat3 NormalToWorldMat;
uniform vec3 ViewOrigin;
uniform float Inter;

attribute vec4 Vert2;
attribute vec3 VertNormal;
attribute vec3 Vert2Normal;
attribute vec2 TexCoord;

varying vec3 Normal;
varying vec3 VertToView;
varying vec3 VPos;
varying vec2 TextureCoordinate;
varying float PlaneDist;
varying float Dist;

void main ()
{
	vec4 Vert;

	Vert = (mix (gl_Vertex, Vert2, Inter) * ModelToWorldMat);
	gl_Position = (gl_ModelViewProjectionMatrix * Vert);
	Normal = (NormalToWorldMat * mix (VertNormal, Vert2Normal, Inter));
	float SurfDist;

	SurfDist = dot (Normal, Vert.xyz);
	PlaneDist = SurfDist;
	Dist = (dot (ViewOrigin, Normal) - SurfDist);
	VertToView = (ViewOrigin - Vert.xyz);
	VPos = (ViewOrigin - gl_Position.xyz);
	TextureCoordinate = TexCoord;
}